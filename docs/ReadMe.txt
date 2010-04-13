-------------------------------------------------------------------------
 DataTrap v3.2.2.59 - http://www.uwom.de - datatrap@uwom.de - Open Source
 Written and Copyrighted © 2008-2010 by WarHead - United Worlds of MaNGOS
-------------------------------------------------------------------------

With this program everyone should understand the original data of
the World of Warcraft®™ v3.3.3.11723 client.

This program is Win32 console based. If you have no idea what this 
means you should better delete this tool from your HardDisk. It
doesn't have a nice colored GUI with funny sounds. In our meaning
that was not necessary for a tool like this.

E.g. you may be able to extract the DBCs (in a user-friendy way!)
and import/export them to a MySQL database. At least this feature
is not the only one, you may try it your way and find out.

Requires Trinitycore Rev >= 7834.

-------------------------------------------------------------------------
Function overview from DataTrap - Help:
-------------------------------------------------------------------------

Usage: DataTrap -switch value (more as one pair is possible)

'-h'                   - this commandline help
'-c'                   - the current configuration data

'-a tdb'               - the db name of your world db (default: tdb)
'-d dbcdata'           - the db name for dbc data (default: dbcdata)
'-w wdbdata'           - the db name for wdb data (default: wdbdata)

'-p password'          - the database password
'-u user'              - the database username
'-s 127.0.0.1'         - the database server ip (default: 127.0.0.1)

'-m c:\mysql\bin'      - path to your mysql binaries
                         it's only important for windows and '-k'
                         if mysql isn't in the system path environment.
'-f c:\path\wow'       - path to your root wow directory

'-e'                   - only extract the dbc files

'-i 2'                 - only import dbc files into your database
                         1 = extract and import dbcs into your database
                         2 = import dbcs into your database
                         3 = import wdbs into your database
        
'-k 4'                 - dump the dbc database
                         4 = dump dbc database
                         5 = dump wdb database
                         6 = dump both

'-insert quest item'   - created sql insert files for new quests and
                         items. possible values: creature,
                         gameobject, quest, item, npctext, pagetext

'-update quest item'   - created sql update files for new quest and
                         item information. possible values: creature,
                         gameobject, quest, item, npctext, pagetext

'-column quest item'   - created sql update files for new quest and
                         item information. possible values: creature,
                         game object, quest, item, npctext, pagetext.
                         Unlike '-update' this must be in the
                         appropriate files in the directory 'misc',
                         the column update the database are
                         specified. the specification of the columns
                         in these files must be with a space
                         separated from each other, and it is pay
                         attention to the spelling (case sensitive).
                         the columns must all be listed in the first
                         row behind the other.

NOTE1: You can use -insert/-update combined but without other
       switches!
NOTE2: You have to write pathnames with spaces
       like this: 'c:\my space path\'!
-------------------------------------------------------------------------

To import something in a MySQL database you must at least indicate 
your username and password. You don't need to define where to find
the "mysqldump.exe" if your "mysqldump.exe" is located in the system
path. If you want to import DBC/WDB files you must put them in the
child folders ./dbc and ./wdb of the DataTrap root directory.

INFO: There are only a view columns descripted at the moment at the
INFO: DBC tables. In one next version there will me more columns
INFO: descripted / more descriptions will be added. But it's a BIG
INFO: write work to do this for so much tables/columns... :-(

ATTENTION: The given DBC database (default: 'dbcdata') will be
ATTENTION: deleted at the import of DBC files!

ATTENTION: The given WDB database (default: 'wdbdata') will only be
ATTENTION: correctly created if it doesn't exists. At the import of
ATTENTION: new WDB files old data will be replaced with the SQL
ATTENTION: command 'REPLACE INTO' and missing data will be added.

To extract DBC/maps DataTrap must be located in the root directory
of World of Warcraft. Otherwise you have to define the path to this 
directory.

All file output will be stored in their corresponding child folder
(sql,dbc,maps,dumps) of the DataTrap root directory. This folders
will be created at the first start.

The -insert/-update parameter don't write/change any data of your
world database. It creates only sql files in the ./sql dir of
DataTrap. You can modify these files as you like and import them
with the batch file (executesql.cmd) which is also created by
DataTrap.

Of course MySQL has to be installed in order to use all functions
of Datatrap dealing with databases.

Thanks to Qubix for his help.

-------------------------------------------------------------------------
And now have fun while watching behind the scenes!
-------------------------------------------------------------------------
