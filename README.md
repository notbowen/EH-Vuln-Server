# Cure51 CGI Server

This is a CGI web server that is vulnerable to a buffer overflow.
I'll write about the steps to compile and run this binary later.

UPDATE: While it is vulnerable to a buffer overflow, I could not
find a reliable way to exploit it without making the entire application
screw itself over :(

Maybe I'll come back to this when my pwn skills get better.

## Structure

CGI Login Binary <-> LDAP Server

## Notes

1. User sends username & password
2. Binary authenticates with LDAP server
3. If the authentication is successful, a directory's file
   content is sent back
4. If the authentication is unsuccessful, an error message is returned
5. A vulnerable debug parameter is set
