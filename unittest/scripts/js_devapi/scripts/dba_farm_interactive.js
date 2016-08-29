// Assumptions: ensure_schema_does_not_exist is available
// Assumes __uripwd is defined as <user>:<pwd>@<host>:<plugin_port>
// validateMemer and validateNotMember are defined on the setup script

dba.dropMetadataSchema({ enforce: true });
var Cluster = dba.createCluster('devCluster', 'testing');

//@ Cluster: validating members

var members = dir(Cluster);

print("Cluster Members:", members.length);
validateMember(members, 'name');
validateMember(members, 'getName');
validateMember(members, 'adminType');
validateMember(members, 'getAdminType');
validateMember(members, 'addSeedInstance');
validateMember(members, 'addInstance');
validateMember(members, 'removeInstance');
validateMember(members, 'getReplicaSet');

//@ Cluster: addInstance, no seed instance answer no
Cluster.addInstance();

//@ Cluster: addInstance, no seed instance answer yes
Cluster.addInstance(5);

//@ Cluster: addInstance, ignore invalid attributes no ignore
Cluster.addInstance({host: __host, data:'sample', port:__mysql_port, whatever:5}, __pwd);

//@ Cluster: addInstance, ignore invalid attributes ignore
Cluster.addInstance({host: __host, data:'sample', port:__mysql_port, whatever:5}, __pwd);

//@ Cluster: addSeedInstance, it already initialized, answer no
Cluster.addSeedInstance({host: __host, port:__mysql_port}, __pwd);

//@ Cluster: addSeedInstance, it already initialized, answer yes
Cluster.addSeedInstance({host: __host, port:__mysql_port}, __pwd);

// Cleanup
dba.dropCluster('devCluster', {dropDefaultReplicaSet: true});