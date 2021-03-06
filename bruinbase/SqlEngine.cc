/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <iostream>
#include <fstream>
#include "Bruinbase.h"
#include "SqlEngine.h"

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);


RC SqlEngine::run(FILE* commandline)
{
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}




RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning
  BTreeIndex treeindex;

  vector<SelCond> value_cond;
  vector<SelCond> key_cond;

  RC     rc;
  int    key;     
  string value;
  int    count;
  int    diff;
  bool ifIndex = false;
  int keyMin = INTMIN; //min int
  int keyMax = INTMAX;  //max int
  SelCond tmp;
  //int a;
 //char m_temp[32];
  int comp_value;

  //ready to print out
  fprintf(stdout,"\n");

  for (unsigned i = 0; i < cond.size(); i++)
  {
    //cout<<"cond info: i , attr, comp, value "<<i<<" "<<cond[i].attr<<" "<<cond[i].comp<<" "<<cond[i].value<<endl;
    if (cond[i].attr == 1)
    {
      //cout<<"cond info: i , comp, value "<<i<<" "<<cond[i].comp<<" "<<cond[i].value<<endl;
      comp_value = atoi(cond[i].value);
      //TODO
      switch (cond[i].comp)
      {
            case SelCond::EQ:
            if(comp_value > keyMin)
              keyMin = comp_value;
            if(comp_value + 1 < keyMax)
              keyMax = comp_value + 1;

        //   //cout<<"Hello Yeah "<<endl;
        // //strcpy(tmp.value, cond[i].value);
        //  tmp.comp = SelCond::LT;
        //  a = atoi(cond[i].value); a++;
        //  // char * m_temp;
        //  sprintf(m_temp, "%d", a);
        //  strcpy(tmp.value, m_temp);
        //  key_cond.push_back(tmp);
        //  //cout << "LT a: " << a;


        // tmp.comp = SelCond::GE;
        // a = atoi(tmp.value);
        // //char * m_temp_1;
        //  sprintf(m_temp, "%d", a);
        //  strcpy(tmp.value, m_temp);
        // key_cond.push_back(tmp);
        // //cout << "GE a: " << a;
        break;

            case SelCond::NE:
        // value_cond.push_back(cond[i]);
            key_cond.push_back(cond[i]);//new change
            //value_cond.push_back(cond[i]);//new change
        break;

            case SelCond::GT:
        //     //cout<<"here "<<endl;
        // //TODO
        // //strcpy(tmp.value, cond[i].value);
        // tmp.comp = SelCond::GE;
        // //a = atoi(cond[i].value); a++;
        // //sprintf(m_temp, "%d", a);
        // //strcpy(tmp.value, m_temp);
        // key_cond.push_back(tmp);
            if(comp_value + 1 > keyMin)
              keyMin = comp_value + 1;


        break;
            case SelCond::LT:
        // key_cond.push_back(cond[i]);
        // //cout<< cond[i].comp<<endl;
            if(comp_value < keyMax)
              keyMax = comp_value;
        break;
            case SelCond::GE:
        // key_cond.push_back(cond[i]);
            if(comp_value > keyMin)
              keyMin = comp_value;
        break;
            case SelCond::LE:
        //strcpy(tmp.value, cond[i].value);
        // tmp.comp = SelCond::LT;
        // a = atoi(cond[i].value); a++;
        // sprintf(m_temp, "%d", a);
        // strcpy(tmp.value, m_temp);
        // key_cond.push_back(tmp);
            if(comp_value + 1 < keyMax)
              keyMax = comp_value + 1;
        break;
      }
    }
    else
      value_cond.push_back(cond[i]);
  }

  // for (unsigned i = 0; i < key_cond.size(); i++)
  // {
  //   if (key_cond[i].comp == SelCond::LT)
  //   {
  //     if (atoi(key_cond[i].value) < keyMax)
  //       keyMax = atoi(key_cond[i].value);
  //   }
  //   if (key_cond[i].comp == SelCond::GE)
  //   {
  //     if (atoi(key_cond[i].value) > keyMin)
  //       keyMin = atoi(key_cond[i].value);
  //   }
  // }

  //cout << "key min: " << keyMin << endl;
  //cout << "key max: " << keyMax << endl;

  //if (keyMin >= keyMax) return 0;
  if (keyMin >= keyMax)
  {
    if(attr == 4)//if it for count(*)
    {
      fprintf(stdout, "%d\n", 0);
    }
    return 0;
  }
  //new change

  int getMin, getMax, getCount;
  if ((rc = treeindex.open(table + ".idx", 'r')) == 0)
  {
    rc = treeindex.readInfo(getMin, getMax, getCount);
    //if ((key_cond.size() != 0) && !(keyMin <= getMin && keyMax >= getMax))
    if(
        !(keyMin <= getMin && keyMax >= getMax) ||
        ((attr == 4 || attr == 1) && value_cond.empty())//no need to read value
      )//new change
      ifIndex = true;
    if (keyMax<= getMin || keyMin>= getMax + 1) 
    {
      if(attr == 4)//if it for count(*)
      {
        fprintf(stdout, "%d\n", 0);
      }
      treeindex.close();
      return 0;
    }
    if(attr == 4 && key_cond.empty() && value_cond.empty() && (keyMin <= getMin) &&(keyMax > getMax))
      {
        fprintf(stdout, "%d\n", getCount);
        treeindex.close();
        return 0;//end the select
      }

  }
  else
    ifIndex = false;
    
  //cout << "get min: " << getMin << endl;
  //cout << "get max: " << getMax << endl;
  //cout << "ifIndex: " << ifIndex << endl;
  //cout << "mark condition "<<((attr == 4 || attr == 1) && value_cond.empty())<<endl;
  //use non-index-based select
  if (!ifIndex)
  {
    // open the table file
    if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
      fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
      return rc;
    }

    // scan the table file from the beginning
    rid.pid = rid.sid = 0;
    count = 0;
    while (rid < rf.endRid()) {
      // read the tuple
      if ((rc = rf.read(rid, key, value)) < 0) {
        fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
        goto exit_select;
      }

      // check the conditions on the tuple
      for (unsigned i = 0; i < cond.size(); i++) {
        // compute the difference between the tuple value and the condition value
        switch (cond[i].attr) {
        case 1:
  	diff = key - atoi(cond[i].value);
  	break;
        case 2:
  	diff = strcmp(value.c_str(), cond[i].value);
  	break;
        }

        // skip the tuple if any condition is not met
        switch (cond[i].comp) {
        case SelCond::EQ:
  	if (diff != 0) goto next_tuple;
  	break;
        case SelCond::NE:
  	if (diff == 0) goto next_tuple;
  	break;
        case SelCond::GT:
  	if (diff <= 0) goto next_tuple;
  	break;
        case SelCond::LT:
  	if (diff >= 0) goto next_tuple;
  	break;
        case SelCond::GE:
  	if (diff < 0) goto next_tuple;
  	break;
        case SelCond::LE:
  	if (diff > 0) goto next_tuple;
  	break;
        }
      }

      // the condition is met for the tuple. 
      // increase matching tuple counter
      count++;

      // print the tuple 
      switch (attr) {
      case 1:  // SELECT key
        fprintf(stdout, "%d\n", key);
        break;
      case 2:  // SELECT value
        fprintf(stdout, "%s\n", value.c_str());
        break;
      case 3:  // SELECT *
        fprintf(stdout, "%d '%s'\n", key, value.c_str());
        break;
      }

      // move to the next tuple
      next_tuple:
      ++rid;
    }

    // print matching tuple count if "select count(*)"
    if (attr == 4) {
      fprintf(stdout, "%d\n", count);
    }
    rc = 0;
    rf.close();
  }

  //use index-based select
  else
  {
    IndexCursor cursor;
    IndexCursor start_cursor;
    IndexCursor end_cursor;

    //cout<<"mark comes to the index"<<endl;
    // scan the table file from the beginning
    rid.pid = rid.sid = 0;
    count = 0;

    rc = treeindex.locate(keyMin, start_cursor);
    if (rc != 0) return rc;
    //cout << "start cursor: " << start_cursor.pid << "  start curssor:  " << start_cursor.eid << endl;
    rc = treeindex.locate(keyMax, end_cursor);
    //cout << "end cursor: " << end_cursor.pid << "  end curssor:  " << end_cursor.eid << endl;
    //rc = treeindex.readForward(cursor, key, rid);
    //if (rc != 0) return rc;


    // for( cursor.pid= start_cursor.pid, cursor.eid = start_cursor.eid;
    //   cursor.pid != end_cursor.pid || cursor.eid != end_cursor.eid; )
    // {
    //   if(rc = treeindex.readForward(cursor, key, rid))
    //     return rc;
    //   //cout<<"info: key, pid, sid are "<<key<<" "<<cursor.pid<<" "<<cursor.eid<<endl;

      //new change here to reduce IO
      //when select key or select count, and value condition is empty
      //cout<<"attr is "<<attr<<endl;

    if((attr == 4 || attr == 1) && value_cond.empty())//no need to go to read out value
    {
      //cout<<"mark to use index to get count"<<endl;
      for( cursor.pid= start_cursor.pid, cursor.eid = start_cursor.eid;
        cursor.pid != end_cursor.pid || cursor.eid != end_cursor.eid; )
      {
        //cout<<"mark before readForward"<<endl;
        if(rc = treeindex.readForward(cursor, key, rid))
        {
          //cout<<"error code is "<<rc<<endl;
          return rc;
        }
        //cout<<"mark after readForward"<<endl;
        //cout<<"info: key, pid, sid are "<<key<<" "<<cursor.pid<<" "<<cursor.eid<<endl;
        //cout<<"mark here dare you come here !"<<endl;

        for(unsigned i = 0; i < key_cond.size(); i++)
        {
          //no switch (key_cond[i].attr) here , because all are key cond
          diff = key - atoi(key_cond[i].value);
          //no switch key_cond[i].comp here, because all are NE comp
          //assert(key_cond[i].comp == SelCond::NE);
          if (diff == 0) goto next_tuple1;//condition not met, not count and not print

        }
        //condition met, count ++ and if select key, print out the key
        count ++;
        if(attr == 1)
          fprintf(stdout, "%d\n", key);
        //task finish, go to next tuple0
        next_tuple1:
        ;
      }
      //cout<<"mark comes here"<<endl;
      if (attr == 4) 
      {
        fprintf(stdout, "%d\n", count);
      }
      treeindex.close();

    }

    else//either select * or select value or value condition is not empty
    {
      // open the table file
      //cout<<"mark here "<<endl;
      if ((rc = rf.open(table + ".tbl", 'r')) < 0) 
      {
        fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
        //cout<<"mark rc "<< rc<<endl;
        return rc;
      }


      for( cursor.pid= start_cursor.pid, cursor.eid = start_cursor.eid;
        cursor.pid != end_cursor.pid || cursor.eid != end_cursor.eid; )
      {
        if(rc = treeindex.readForward(cursor, key, rid))
          return rc;
        //cout<<"info: key, pid, sid are "<<key<<" "<<cursor.pid<<" "<<cursor.eid<<endl;

        if ((rc = rf.read(rid, key, value)) < 0) 
        {
          fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
          goto exit_select;
        }

        // check the conditions on the tuple, first for key_cond
        for(unsigned i = 0; i < key_cond.size(); i++)
        {
          //no switch (key_cond[i].attr) here , because all are key cond
          diff = key - atoi(key_cond[i].value);
          //no switch key_cond[i].comp here, because all are NE comp
          if (diff == 0) goto next_tuple0;//condition not met, not count and not print
        }
        // check the conditions on the tuple, seconc for value_cond
        //cout << "size: " << value_cond.size() << endl;
        for (unsigned i = 0; i < value_cond.size(); i++) 
        {
          // compute the difference between the tuple value and the value_condition value
          diff = strcmp(value.c_str(), value_cond[i].value);

            //cout << "diff: " << diff << endl;
            //cout << "comp: " << value_cond[i].comp << endl;
          // skip the tuple if any value_condition is not met
          switch (value_cond[i].comp) 
          {
                case SelCond::EQ:
            if (diff != 0) goto next_tuple0;
            break;
                case SelCond::NE:
            if (diff == 0) goto next_tuple0;
            break;
                case SelCond::GT:
            if (diff <= 0) goto next_tuple0;
            break;
                case SelCond::LT:
            if (diff >= 0) goto next_tuple0;
            break;
                case SelCond::GE:
            if (diff < 0) goto next_tuple0;
            break;
                case SelCond::LE:
            if (diff > 0) goto next_tuple0;
            break;
          }
        }
          // the condition is met for the tuple. 
          // increase matching tuple counter
        count++;

            // print the tuple 
        switch (attr) 
        {
            case 1:  // SELECT key
              fprintf(stdout, "%d\n", key);
              break;
            case 2:  // SELECT value
              fprintf(stdout, "%s\n", value.c_str());
              break;
            case 3:  // SELECT *
              fprintf(stdout, "%d '%s'\n", key, value.c_str());
              break;
        }
      
        next_tuple0:
        ;  
      }
    
        //rc = treeindex.readForward(cursor, key, rid);
      //}

       // print matching tuple count if "select count(*)"
      if (attr == 4) 
      {
        fprintf(stdout, "%d\n", count);
      }
      rc = 0;
      //cout << endl;
      rf.close();
      treeindex.close();
    }
  }


  // close the table file and return
  exit_select:
  
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  //file --> parseLoadLine --> load
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning
  ifstream infile(loadfile.c_str());  //Read file; Must use c_str() since it's C++03
  BTreeIndex treeindex;

  RC     rc;
  int    key;     
  string line, value;

  if (index == true)
  {
    rc = treeindex.open(table + ".idx", 'w');
    ////cout << "mark 0" << endl;
    if (rc != 0) {treeindex.close(); return rc;}  
  }

  //Open the table file
  //Open a file in read or write mode. (r/w)
  //When opened in 'w' mode, if the file does not exist, it is created.
  rc = rf.open(table + ".tbl", 'w');
  if (rc !=0) return rc;

    //Get each line from loadfile
  while (getline(infile, line))
  {
    rid = rf.endRid();
    //Get key and value by using parseLoadLine()
    //Get the last record id. Note rid is an instant of class RecordId
    parseLoadLine(line, key, value);
    rc = rf.append(key, value, rid);
    ////cout << "mark 1" << endl;
    if (rc != 0) return rc;
    if (index == true)
    {
      //rc = treeindex.open(table + ".idx", 'w');
      //if (rc != 0) return rc;  
      rc = treeindex.insert(key, rid);
      ////cout << "mark 2" << endl;
      if (rc != 0) return rc;
      //treeindex.close();
    }
  }

  //Close the table file
  rf.close();
  //Close the index file
  if (index == true) treeindex.close();
  return 0;
}

RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
    const char *s;
    char        c;
    string::size_type loc;
    
    // ignore beginning white spaces
    c = *(s = line.c_str());
    while (c == ' ' || c == '\t') { c = *++s; }

    // get the integer key value
    key = atoi(s);

    // look for comma
    s = strchr(s, ',');
    if (s == NULL) { return RC_INVALID_FILE_FORMAT; }

    // ignore white spaces
    do { c = *++s; } while (c == ' ' || c == '\t');
    
    // if there is nothing left, set the value to empty string
    if (c == 0) { 
        value.erase();
        return 0;
    }

    // is the value field delimited by ' or "?
    if (c == '\'' || c == '"') {
        s++;
    } else {
        c = '\n';
    }

    // get the value string
    value.assign(s);
    loc = value.find(c, 0);
    if (loc != string::npos) { value.erase(loc); }

    return 0;
}
