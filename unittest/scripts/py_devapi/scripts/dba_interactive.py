# Assumptions: ensure_schema_does_not_exist is available
# Assumes __uripwd is defined as <user>:<pwd>@<host>:<plugin_port>
# validateMemer and validateNotMember are defined on the setup script

#@ Initialization
dba.drop_metadata_schema({'enforce':True})

#@ Session: validating members
all_members = dir(dba)

# Remove the python built in members
members = []
for member in all_members:
  if not member.startswith('__'):
    members.append(member)


print "Session Members: %d" % len(members)
validateMember(members, 'default_cluster')
validateMember(members, 'get_default_cluster')
validateMember(members, 'create_cluster')
validateMember(members, 'drop_cluster')
validateMember(members, 'get_cluster')
validateMember(members, 'drop_metadata_schema')
validateMember(members, 'reset_session')
validateMember(members, 'validate_instance')
validateMember(members, 'deploy_local_instance')

#@# Dba: create_cluster errors
cluster = dba.create_cluster()
cluster = dba.create_cluster(5)
cluster = dba.create_cluster('')

#@# Dba: create_cluster with interaction
cluster = dba.create_cluster('devCluster')
print cluster

#@# Dba: get_cluster errors
cluster = dba.get_cluster()
cluster = dba.get_cluster(5)
cluster = dba.get_cluster('', 5)
cluster = dba.get_cluster('')
cluster = dba.get_cluster('devCluster')

#@ Dba: get_cluster
print cluster

#@ Dba: add_seed_instance
cluster.add_seed_instance({'host': __host, 'port':__mysql_port}, __pwd)

#@# Dba: drop_cluster errors
cluster = dba.drop_cluster()
cluster = dba.drop_cluster(5)
cluster = dba.drop_cluster('')
cluster = dba.drop_cluster('sample', 5)
cluster = dba.drop_cluster('sample', {}, 5)

#@ Dba: drop_cluster interaction no options, cancel
cluster = dba.drop_cluster('sample')

#@ Dba: drop_cluster interaction missing option, ok error
cluster = dba.drop_cluster('sample', {})

#@ Dba: drop_cluster interaction no options, ok success
cluster = dba.drop_cluster('devCluster')