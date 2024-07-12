#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ldap.h>
#include <ctype.h>
#include <unistd.h>

#define LDAP_HOST "ldap://192.168.0.15" // TODO: Change hard coded IP
#define LDAP_PORT 389
#define LDAP_BASE_DN "DC=cure51,DC=com"

bool contains_char(char c, char *list)
{
    while (*list != '\0')
    {
        if (c == *list)
            return true;
        list++;
    }

    return false;
}

int get_string(char *dest, char *buf, int start, int length, char *breakChar)
{
    int cur = start;
    while (cur < length && !contains_char(buf[cur], breakChar))
        cur++;
    strncpy(dest, buf + start, cur - start);
    dest[cur - start] = '\0';
    return cur;
}

void url_decode(char *src, char *dest)
{
    char *p = src;
    char code[3] = {0};
    while (*p)
    {
        if (*p == '%')
        {
            memcpy(code, ++p, 2);
            *dest++ = (char)strtoul(code, NULL, 16);
            p += 2;
        }
        else if (*p == '+')
        {
            *dest++ = ' ';
            p++;
        }
        else
        {
            *dest++ = *p++;
        }
    }
    *dest = '\0';
}

char *html =
    "\
    <!DOCTYPE html>\n\
    <html>\n\
        <head>\n\
            <title>Login</title>\n\
        </head>\n\
        <body>\n\
            %s\n\
        </body>\n\
    </html>\n";

char *alert_and_return =
    "\
    <script>\n\
        alert(\"%s\");\n\
        window.location.href = \"/\";\n\
    </script>\n";

void return_html(char *output)
{
    printf(html, output);
}

void return_alert(char *output)
{
    printf(alert_and_return, output);
}

// Login to LDAP with the username & password
bool verify(char *username, char *password)
{
    LDAP *ld;
    int rc;
    char decoded_username[256], decoded_password[256];

    url_decode(username, decoded_username);
    url_decode(password, decoded_password);

    if (decoded_username[0] == '\0' || decoded_password[0] == '\0') {
        return false;
    }

    rc = ldap_initialize(&ld, LDAP_HOST);
    if (rc != LDAP_SUCCESS)
    {
        fprintf(stderr, "ldap_initialize() failed: %d\n", rc);
        return false;
    }

    rc = ldap_simple_bind_s(ld, decoded_username, decoded_password);
    if (rc != LDAP_SUCCESS)
    {
        fprintf(stderr, "ldap_simple_bind_s() failed: %d\n", rc);
        return false;
    }

    return true;
}

void login()
{
    char buf[512];
    char username[128] = {0};
    char password[128] = {0};
    int length;

    gets(buf);
    length = strlen(buf);

    int start = 0;

    while (start < length)
    {
        char data[128];
        char name[64];

        start = get_string(name, buf, start, length, "=&");
        if (start >= length || buf[start] == '&')
        {
            data[0] = '\0';
            start++;
        }
        else
        {
            start = get_string(data, buf, start + 1, length, "&") + 1;
        }

        if (strcmp(name, "username") == 0)
            sprintf(username, data);
        else if (strcmp(name, "password") == 0)
            strcpy(password, data);
    }

    if (!verify(username, password)) {
        printf("<h1>Invalid username(");
        printf(username);
        printf(") or password</h1>");

        printf(alert_and_return, "Invalid username or password");
        return;
    }

    printf("<html><body><h1>Welcome, ");
    printf(username);
    printf("</h1><pre>");

    FILE *fp = popen("/bin/bash ./read-files.sh", "r");
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), fp))
    {
        printf("%s", buffer);
    }
    pclose(fp);

    printf("</pre></body></html>\n");
}

int main()
{
    printf("Content-type: text/html\n\n");
    login();
    return 0;
}
