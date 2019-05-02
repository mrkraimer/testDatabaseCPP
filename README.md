# testDatabaseCPP

This contains code to test IOC databases.

In particular it has an exampleDatabase that adds to epics-base records for every possible DBF type.

It also has example that has client code to tests every record type in the exampleDatabase.


## Building

If a proper RELEASE.local file exists one directory levels above **testClientCPP**, then just type:

    make

It can also be built by:

    cp configure/ExampleRELEASE.local configure/RELEASE.local
    edit file configure/RELEASE.local
    make

In **configure/RELEASE.local** it may only be necessary to change the definitions
of **EPICS4_DIR** and **EPICS_BASE**.

## DBRecordTypes test

### Start IOC for test

    mrk> pwd
    /home/epicsv4/masterCPP/testDatabaseCPP/iocBoot/DBRecordTypes
    mrk> ../../bin/linux-x86_64/recordTypes st.cmd


### Run DBRrecordTypesClient

    mrk> pwd
    /home/epicsv4/masterCPP/testDatabaseCPP
    mrk> bin/linux-x86_64/DBRrecordTypesClient -p ca
    diff temp.txt ca.txt
    mrk> bin/linux-x86_64/DBRrecordTypesClient -p pva
    diff temp.txt pva.txt

**NOTE** the last diff will show changes in time.


### Results.

Look at ca.txt and pva.txt

## manyChannels test

This is a tests that starts an IOC with 50,000 DBRecords
and a clients that provides a combination of channelGet, channelPut, and monitor.

### Start IOC for test

    mrk> pwd
    /home/epicsv4/masterCPP/testDatabaseCPP/iocBoot/manyChannels
    mrk> ../../bin/linux-x86_64/recordTypes st.cmd

### Run clients

A number of client tests are available.
Amoung these are **timeManyChannel**, **timeManyChannelGet**, **timeManyChannelMonitor**, 
**timeManyChannelPut**, and **timeManyChannelGPM**.

Each supports options for the numnber of channels and the provider.

For example:

    mrk> pwd
    /home/epicsv4/masterCPP/testDatabaseCPP
    mrk> bin/linux-x86_64/timeManyChannel -h
    -p provider -n nchannels  
    default
    -p pva -n 50000

**WARNING** These tests can use  significent amount of memory,
You might want to run a system monitor to see that you do not use up all your memory.


An example of running a test is:


    mrk> pwd
    /home/epicsv4/masterCPP/testDatabaseCPP
    mrk> bin/linux-x86_64/timeManyChannel

There are also other examples: **getmany**, **putmany**, and **monitormany**.
Look at the code for semantics.

## fastCalc test.

This is a test that has calc records that are processing as fast as they can.

### Start IOC for test

    mrk> pwd
    /home/epicsv4/masterCPP/testDatabaseCPP/iocBoot/fastcalc
    mrk> ../../bin/linux-x86_64/recordTypes st.fast1
    OR
    mrk> ../../bin/linux-x86_64/recordTypes st.fast1_4


### Run clients

Use any combination of **caget**, **camonitor**, and **pvget** to test.



