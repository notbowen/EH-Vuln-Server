FROM httpd:latest
RUN apt-get update && apt-get install -y gcc

# Copy HTML page
COPY htdocs/index.html /usr/local/apache2/htdocs/

# Configure Apache
RUN echo "\
<Directory /usr/local/apache2/htdocs> \n\
    Options Indexes FollowSymLinks \n\
    AllowOverride None \n\
    Require all granted \n\
</Directory> \n\
" >> /usr/local/apache2/conf/httpd.conf

# Copy CGI script
COPY cgi-bin/login.c /usr/local/apache2/cgi-bin/

# Compile CGI script
WORKDIR /usr/local/apache2/cgi-bin/
RUN gcc -o login.cgi login.c
RUN chmod 755 login.cgi

# Configure Apache
RUN echo "\
LoadModule cgid_module modules/mod_cgid.so \n\
<Directory /usr/local/apache2/cgi-bin> \n\
    Options +ExecCGI \n\
    AddHandler cgi-script .cgi \n\
</Directory> \n\
" >> /usr/local/apache2/conf/httpd.conf

EXPOSE 80
CMD ["httpd-foreground"]
