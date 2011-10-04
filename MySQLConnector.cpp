/* 
 * File:   MySQLConnector.cpp
 * Author: olmin
 * 
 * Created on 22 de septiembre de 2011, 14:51
 */

#include "MySQLConnector.h"

/* Standard C++ headers */
#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <stdexcept>
#include <sys/time.h>

/* MySQL Connector/C++ specific headers */
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/metadata.h>
#include <cppconn/resultset_metadata.h>
#include <cppconn/exception.h>
#include <cppconn/warning.h>

#define DBHOST "tcp://127.0.0.1:3306"
#define USER "olmin"
#define PASSWORD "dover"
#define DATABASE "behavioursdb"

#define NUMOFFSET 100
#define COLNAME 200

using namespace std;
using namespace sql;

MySQLConnector::MySQLConnector() {

    try {
        driver = get_driver_instance();

        /* alternate syntax using auto_ptr to create the db connection */
        //auto_ptr  con (driver -> connect(url, user, password));

    } catch (SQLException &e) {
        cout << "ERROR: SQLException in " << __FILE__;
        cout << " (" << __func__ << ") on line " << __LINE__ << endl;
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;

        if (e.getErrorCode() == 1047) {
            /*
            Error: 1047 SQLSTATE: 08S01 (ER_UNKNOWN_COM_ERROR)
            Message: Unknown command
             */
            cout << "\nYour server does not seem to support Prepared Statements at all. ";
            cout << "Perhaps MYSQL < 4.1?" << endl;
        }

        //return EXIT_FAILURE;
    } catch (std::runtime_error &e) {

        cout << "ERROR: runtime_error in " << __FILE__;
        cout << " (" << __func__ << ") on line " << __LINE__ << endl;
        cout << "ERROR: " << e.what() << endl;

        //return EXIT_FAILURE;
    }
    //return EXIT_SUCCESS;
}

MySQLConnector::MySQLConnector(const MySQLConnector& orig) {
}

MySQLConnector::~MySQLConnector() {
}

/*!Returns the next unique id to be inserted in the db's row*/
static int getNextId(Connection *con){
    ResultSet *res;
    Statement *stmt;      
     
    /* create a statement object */
    stmt = con -> createStatement();
    /* run a query which returns exactly one result set */
    res = stmt -> executeQuery ("SELECT * FROM Behaviour");
    int lastId= res -> rowsCount();     
     
    //cleanup
    delete res;
    delete stmt;
        
    return lastId+1;        
}

void MySQLConnector::save(Behaviour *behaviour) {

    Connection *con=NULL;   
    PreparedStatement *prep_stmt=NULL;
    Savepoint *savept=NULL;

    int updatecount = 0;
    char dateBuffer [22];
    
    try {
        /* create a database connection using the Driver */
        con = driver -> connect(DBHOST, USER, PASSWORD);
        
        /* select appropriate database schema */
	con -> setSchema(DATABASE);
        
        int idNewBehaviour= getNextId(con);

        /* turn off the autocommit */
        con -> setAutoCommit(false);          
        cout << "\t Autocommit false .." << endl;             
        
        cout << "\tCreating a save point \"SAVEPT1\" .." << endl;
        savept = con -> setSavepoint("SAVEPT1");
               
        prep_stmt = con -> prepareStatement(
                "INSERT INTO `Behaviour` (`id`, `name`, `date_ini`, `date_end`, `frames`) "
                "VALUES (?, ?, ?, ?, ?);");
        
        prep_stmt -> setInt(1,idNewBehaviour); 
        prep_stmt -> setString(2, behaviour->name);
        
        strftime (dateBuffer,22,"%Y-%m-%d %H:%M:%S",localtime (& behaviour->dateIni)); //YYYY-MM-DD HH:MM:SS.microssec        
        prep_stmt -> setDateTime(3, dateBuffer);
              
        strftime (dateBuffer,22,"%Y-%m-%d %H:%M:%S",localtime (& behaviour->dateEnd)); //YYYY-MM-DD HH:MM:SS.microssec       
        prep_stmt -> setDateTime(4, dateBuffer);
        prep_stmt -> setInt(5, behaviour->count);

        cout << "\t Execute update.." << endl;           
        updatecount = prep_stmt -> executeUpdate();
 
        prep_stmt = con -> prepareStatement(
                "INSERT INTO `Skeleton` (`id_behaviour`, `frame`, `pos3D`, `ang0`, `ang1`, `ang2`, `ang3`, `ang4`, `ang5`, `ang6`, `ang7`, `ang8`, `ang9`, `ang10`) "
                "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);"
                );

        for (int i = 0; i < behaviour->count;i++) {
           
            stringstream point3D (stringstream::in | stringstream::out);      
            point3D << "(" << behaviour->positions[i].pos3D.x << ", " 
                    << behaviour->positions[i].pos3D.y << ", " 
                    << behaviour->positions[i].pos3D.z << ")";
                    
            prep_stmt -> setInt(1, idNewBehaviour);
            prep_stmt -> setInt(2, i+1);
            prep_stmt -> setString(3,point3D.str());
            prep_stmt -> setDouble(4, behaviour->positions[i].angles[0]);
            prep_stmt -> setDouble(5, behaviour->positions[i].angles[1]);
            prep_stmt -> setDouble(6, behaviour->positions[i].angles[2]);
            prep_stmt -> setDouble(7, behaviour->positions[i].angles[3]);
            prep_stmt -> setDouble(8, behaviour->positions[i].angles[4]);
            prep_stmt -> setDouble(9, behaviour->positions[i].angles[5]);
            prep_stmt -> setDouble(10, behaviour->positions[i].angles[6]);
            prep_stmt -> setDouble(11, behaviour->positions[i].angles[7]);
            prep_stmt -> setDouble(12, behaviour->positions[i].angles[8]);
            prep_stmt -> setDouble(13, behaviour->positions[i].angles[9]);
            prep_stmt -> setDouble(14, behaviour->positions[i].angles[10]);            
               
           updatecount = prep_stmt -> executeUpdate();
           
        }      
        
        cout << "\tCommitting outstanding updates to the database .." << endl;
        con -> commit();

        /* Clean up */      
        delete prep_stmt;
        con -> close();
        delete con;

    } catch (SQLException &e) {
        if (con != NULL && savept!= NULL) {
            cout << "\tRolling back until the last save point \"SAVEPT1\" .." << endl;
            con -> rollback(savept);
            con -> releaseSavepoint(savept);              
            cout << "\t Rollback done.." << endl;
        }

        cout << "ERROR: SQLException in " << __FILE__;
        cout << " (" << __func__ << ") on line " << __LINE__ << endl;
        cout << "ERROR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << ")" << endl;

        if (e.getErrorCode() == 1047) {
            /*
            Error: 1047 SQLSTATE: 08S01 (ER_UNKNOWN_COM_ERROR)
            Message: Unknown command
             */
            cout << "\nYour server does not seem to support Prepared Statements at all. ";
            cout << "Perhaps MYSQL < 4.1?" << endl;
        }

        //return EXIT_FAILURE;
    } catch (std::runtime_error &e) {

        cout << "ERROR: runtime_error in " << __FILE__;
        cout << " (" << __func__ << ") on line " << __LINE__ << endl;
        cout << "ERROR: " << e.what() << endl;

        //return EXIT_FAILURE;
    }
    // return EXIT_SUCCESS;
}

void MySQLConnector::closeConnection() {

}