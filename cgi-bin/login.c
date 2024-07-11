#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ldap.h>
#include <ctype.h>
#include <unistd.h>

#define LDAP_HOST "ldap://192.168.0.15"  // TODO: Change hard coded IP
#define LDAP_PORT 389
#define LDAP_BASE_DN "DC=cure51,DC=com"

// Function to URL decode the input
void url_decode(char *src, char *dest) {
    char *p = src;
    char code[3] = {0};
    while(*p) {
        if(*p == '%') {
            memcpy(code, ++p, 2);
            *dest++ = (char)strtoul(code, NULL, 16);
            p += 2;
        } else if(*p == '+') {
            *dest++ = ' ';
            p++;
        } else {
            *dest++ = *p++;
        }
    }
    *dest = '\0';
}

// Function to get CGI parameters
char *get_cgi_param(char *query_string, const char *param) {
    char *pair;
    char *value;
    char *query = strdup(query_string);
    
    pair = strtok(query, "&");
    while(pair) {
        value = strchr(pair, '=');
        if(value) {
            *value = '\0';
            value++;
            if(strcmp(pair, param) == 0) {
                char *result = strdup(value);
                free(query);
                return result;
            }
        }
        pair = strtok(NULL, "&");
    }
    free(query);
    return NULL;
}

int main() {
    LDAP *ld;
    int rc;
    char *username, *password;
    char bind_dn[32];
    char *query_string;
    char decoded_username[256], decoded_password[256];

    // Prepare the response
    printf("Content-type: text/html\n\n");

    // Get the query string
    query_string = getenv("QUERY_STRING");
    if(!query_string || *query_string == '\0') {
        printf("<html><body><form><p>Username: <input type=\"text\" name=\"username\"></p><p>Password: <input type=\"password\" name=\"password\"></p><p><input type=\"submit\" value=\"Login\"></p></form></body></html>\n");
        return 1;
    }

    // Extract username and password from query string
    username = get_cgi_param(query_string, "username");
    password = get_cgi_param(query_string, "password");

    // URL decode the username and password
    url_decode(username, decoded_username);
    url_decode(password, decoded_password);

    // Initialize LDAP connection
    rc = ldap_initialize(&ld, LDAP_HOST);
    if (rc != LDAP_SUCCESS) {
        printf("Error: ldap_initialize failed: %s\n", ldap_err2string(rc));
        return 1;
    }

    // Set LDAP version to 3
    int version = LDAP_VERSION3;
    ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version);

    // Construct the bind DN vulnerable to buffer overflow
    strcpy(bind_dn, decoded_username);

    // Perform bind operation
    rc = ldap_simple_bind_s(ld, bind_dn, decoded_password);
    if (rc != LDAP_SUCCESS) {
        printf("<html><body><form><p>Username: <input type=\"text\" name=\"username\"></p><p>Password: <input type=\"password\" name=\"password\"></p><p><input type=\"submit\" value=\"Login\"></p><p style=\"color:red\">Login failed: %s</p></form></body></html>\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ld, NULL, NULL);
        return 1;
    }

    // Unbind and close the connection
    ldap_unbind_ext_s(ld, NULL, NULL);

    // Overwrite query string with the command to execute
    snprintf(query_string, strlen(query_string), "./read-files.sh");

    // Print the HTML page displaying the output
    printf("<html><body><h1>Welcome, %s</h1><p>DEBUG: %s</p><pre>", decoded_username, query_string);

    // Run command and get output
    FILE *fp = popen(query_string, "r");
    char buffer[1024];
    while(fgets(buffer, sizeof(buffer), fp)) {
        printf("%s", buffer);
    }
    pclose(fp);

    printf("</pre></body></html>\n");

    return 0;
}
