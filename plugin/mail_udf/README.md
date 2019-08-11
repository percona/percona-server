# Mail UDF

Sends basic emails from MySQL using curl

## Requirements
- libcurl


## Installation
_Requires MySQL v8.0_
```
mysql> INSTALL PLUGIN mail SONAME 'udf_mail.so';
```

## Usage
```
select mail_send('smtp://smtp.domain:port', 'from@address', 'to@adddress', 'cc@address or empty', 'Mail subject', 'Mail body');
select mail_send('smtps://smtp.domain:port', 'from@address', 'to@adddress', 'cc@address or empty', 'Mail subject', 'Mail body');
```
