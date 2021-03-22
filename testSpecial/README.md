# testSpecial

This tests the special support in pvDatabaseCPP

## To start the example

    mrk> pwd
    /home/epics7/modules/testDatabaseCPP/testSpecial/iocBoot/testSpecial
    mrk> ../../bin/linux-x86_64/testSpecial st.cmd 

## testing

   mrk> pwd
   /home/epics7/modules/testDatabaseCPP/testSpecial
   ./testScalar &> temp
   diff temp testScalarResult //THERE SHOULD BE NO DIFFERENCES
   rm temp
   ./testScalarArray &> temp
   diff temp testScalarArrayResult //THERE SHOULD BE NO DIFFERENCES
   rm temp
   ./testAddRemoveProcessTrace &> temp
   diff temp testAddRemoveProcessTraceResult //Only the timStamps should differ
   rm temp
   
   

