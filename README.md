# IniHandler
This c program will read and parse an ini formatted file.  It has all the CRUD (Create, Read, Update, Delete) features and can save to a file, making this a great addition to any program.

*** WARNING ***
Majority of the data is dynamically allocated and has an extended buffer that will grow as needed for a key/value pair.  This code does NOT look for a max line length as of this time, so the extended buffer will grow until the end of the line or eof is found.
