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

## Start IOC for test

    mrk> pwd
    /home/epicsv4/masterCPP/testDatabaseCPP/iocBoot/exampleDatabase
    mrk> ../../bin/linux-x86_64/exampleDatabase st.cmd


## Run example

    mrk> pwd
    /home/epicsv4/masterCPP/testDatabaseCPP/example
    mrk> bin/linux-x86_64/exampleDatabase -p ca
    diff temp.txt ca.txt
    mrk> bin/linux-x86_64/exampleDatabase -p pva
    diff temp.txt pva.txt


## Results.

Look at ca.txt and pva.txt





