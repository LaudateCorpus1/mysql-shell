//@<> Auxiliary Functions from devapi setup.js
function wait(timeout, wait_interval, condition){
  waiting = 0;
  res = condition();
  while(!res && waiting < timeout) {
    os.sleep(wait_interval);
    waiting = waiting + 1;
    res = condition();
  }
  return res;
}

function cleanup_sandbox(port) {
    println ('Stopping the sandbox at ' + port + ' to delete it...');
    try {
      stop_options = {}
      stop_options['password'] = 'root';
      if (__sandbox_dir != '')
        stop_options['sandboxDir'] = __sandbox_dir;

      dba.stopSandboxInstance(port, stop_options);
    } catch (err) {
      println(err.message);
    }

    options = {}
    if (__sandbox_dir != '')
      options['sandboxDir'] = __sandbox_dir;

    var deleted = false;

    print('Try deleting sandbox at: ' + port);
    deleted = wait(10, 1, function() {
      try {
        dba.deleteSandboxInstance(port, options);

        println(' succeeded');
        return true;
      } catch (err) {
        println(' failed: ' + err.message);
        return false;
      }
    });
    if (deleted) {
      println('Delete succeeded at: ' + port);
    } else {
      println('Delete failed at: ' + port);
    }
}

// -------------------------------------------------------------------------------------
// BUG#27329079 - CREATE CLUSTER NOT POSSIBLE IF SERVER STARTED WITH INNODB_PAGE_SIZE=4K
// -------------------------------------------------------------------------------------

//@<> Deploy instances (with specific innodb_page_size). {!real_host_is_loopback}
var __sandbox_dir = testutil.getSandboxPath();
//NOTE: Can not use testutil.deploySandbox here as it will reuse a pre-generated data file that is not
//      compatible with the innodb_page_size used on this test, so we need clean sandboxes generated
//      with the required page size
dba.deploySandboxInstance(__mysql_sandbox_port1, {allowRootFrom:"%", mysqldOptions: ["innodb_page_size=4k", "report_host="+hostname], password: 'root', sandboxDir:__sandbox_dir});
dba.deploySandboxInstance(__mysql_sandbox_port2, {allowRootFrom:"%", mysqldOptions: ["innodb_page_size=8k", "report_host="+hostname], password: 'root', sandboxDir:__sandbox_dir});
EXPECT_STDERR_EMPTY();

var mycnf1 = testutil.getSandboxConfPath(__mysql_sandbox_port1);

//@<> checkInstanceConfiguration error with innodb_page_size=4k. {!real_host_is_loopback}
EXPECT_THROWS(function(){
  dba.checkInstanceConfiguration(__sandbox_uri1, {mycnfPath: mycnf1});
}, "Dba.checkInstanceConfiguration: Unsupported innodb_page_size value: 4096");
EXPECT_OUTPUT_CONTAINS(`ERROR: Instance '${hostname}:${__mysql_sandbox_port1}' is using a non-supported InnoDB page size (innodb_page_size=4096). Only instances with innodb_page_size greater than 4k (4096) can be used with InnoDB Cluster.`);

//@<> configureLocalInstance error with innodb_page_size=4k. {!real_host_is_loopback}
EXPECT_THROWS(function(){
  dba.configureLocalInstance(__sandbox_uri1, {mycnfPath: mycnf1});
}, "Dba.configureLocalInstance: Unsupported innodb_page_size value: 4096");
EXPECT_OUTPUT_CONTAINS(`ERROR: Instance '${hostname}:${__mysql_sandbox_port1}' is using a non-supported InnoDB page size (innodb_page_size=4096). Only instances with innodb_page_size greater than 4k (4096) can be used with InnoDB Cluster.`);


//@<> create cluster fails with nice error if innodb_page_size=4k {!real_host_is_loopback}
shell.connect(__sandbox_uri1);
EXPECT_THROWS(function(){
  var cluster = dba.createCluster("test_cluster", {gtidSetIsComplete: true});
}, "Dba.createCluster: Unsupported innodb_page_size value: 4096");
EXPECT_OUTPUT_CONTAINS(`ERROR: Instance '${hostname}:${__mysql_sandbox_port1}' is using a non-supported InnoDB page size (innodb_page_size=4096). Only instances with innodb_page_size greater than 4k (4096) can be used with InnoDB Cluster.`);

//@<> create cluster works with innodb_page_size=8k (> 4k) {!real_host_is_loopback}
shell.connect(__sandbox_uri2);
var cluster = dba.createCluster("test_cluster", {gtidSetIsComplete: true});
EXPECT_STDERR_EMPTY();

//@<> Clean-up deployed instances. {!real_host_is_loopback}
cleanup_sandbox(__mysql_sandbox_port1);
cleanup_sandbox(__mysql_sandbox_port2);

// -----------------------------------------------------------------------------------------
// BUG#28531271 - CREATECLUSTER FAILS WHEN INNODB_DEFAULT_ROW_FORMAT IS COMPACT OR REDUNDANT
// -----------------------------------------------------------------------------------------
//@<> dba.createCluster using innodb_default_row__format=COMPACT
testutil.deploySandbox(__mysql_sandbox_port1, "root", {report_host: hostname, innodb_default_row_format: 'COMPACT'});
shell.connect(__sandbox_uri1);
var c = dba.createCluster('sample');
EXPECT_STDERR_EMPTY();
session.close();
testutil.destroySandbox(__mysql_sandbox_port1);

//@<> dba.createCluster using innodb_default_row__format=REDUNDANT
testutil.deploySandbox(__mysql_sandbox_port1, "root", {report_host: hostname, innodb_default_row_format: 'REDUNDANT'});
shell.connect(__sandbox_uri1);
var c = dba.createCluster('sample');
EXPECT_STDERR_EMPTY();
session.close();
testutil.destroySandbox(__mysql_sandbox_port1);
