char *html_form =
"<form method=\"POST\" action=\"%s\" enctype=\"multipart/form-data\">"
"<table class='postform'>"
"<tr id='formtitle'><td colspan='3'><span>%s</span></td></tr>"
"<tr><td>Subject:</td><td><input type = \"text\" name = \"input_name\" style='width:100%%'/></td>"
"<td><input type =\"submit\"/></td></tr>"
"<tr><td>E-mail:</td><td colspan='2'><input type = \"text\" name = \"input_email\" style='width:100%%'/></td></tr>"
"<tr><td rowspan='2'>Comment:</td>"
"<td colspan='2'><textarea rows='8' style='width:100%%' name = \"input_content\" id='comment'></textarea></td></tr>"
"<tr><td>"
"<select id='emoji' onchange=\"document.getElementById('comment').value += (this.value);document.getElementById('comment').focus();\">"
"<option value=''>&#26080;</option><option value='&#124;&#8704;&#65439;'>&#124;&#8704;&#65439;</option><option value='&#40;&#180;&#65439;&#1044;&#65439;&#96;&#41;'>&#40;&#180;&#65439;&#1044;&#65439;&#96;&#41;</option><option value='&#40;&#59;&#180;&#1044;&#96;&#41;'>&#40;&#59;&#180;&#1044;&#96;&#41;</option><option value='&#40;&#65344;&#65381;&#969;&#65381;&#41;'>&#40;&#65344;&#65381;&#969;&#65381;&#41;</option><option value='&#40;&#61;&#65439;&#969;&#65439;&#41;&#61;'>&#40;&#61;&#65439;&#969;&#65439;&#41;&#61;</option><option value='&#124;&#32;&#969;&#12539;&#180;&#41;'>&#124;&#32;&#969;&#12539;&#180;&#41;</option><option value='&#124;&#45;&#96;&#32;&#41;'>&#124;&#45;&#96;&#32;&#41;</option><option value='&#124;&#1076;&#96;&#32;&#41;'>&#124;&#1076;&#96;&#32;&#41;</option><option value='&#124;&#12540;&#96;&#32;&#41;'>&#124;&#12540;&#96;&#32;&#41;</option><option value='&#124;&#8704;&#96;&#32;&#41;'>&#124;&#8704;&#96;&#32;&#41;</option><option value='&#40;&#12388;&#1076;&#8834;&#41;'>&#40;&#12388;&#1076;&#8834;&#41;</option><option value='&#40;&#65439;&#1044;&#65439;&#8801;&#65439;&#1044;&#65439;&#41;'>&#40;&#65439;&#1044;&#65439;&#8801;&#65439;&#1044;&#65439;&#41;</option><option value='&#40;&#65342;&#111;&#65342;&#41;&#65417;'>&#40;&#65342;&#111;&#65342;&#41;&#65417;</option><option value='&#40;&#124;&#124;&#124;&#65439;&#1044;&#65439;&#41;'>&#40;&#124;&#124;&#124;&#65439;&#1044;&#65439;&#41;</option><option value='&#40;&#32;&#65439;&#8704;&#65439;&#41;'>&#40;&#32;&#65439;&#8704;&#65439;&#41;</option><option value='&#40;&#32;&#180;&#8704;&#96;&#41;'>&#40;&#32;&#180;&#8704;&#96;&#41;</option><option value='&#40;&#42;&#180;&#8704;&#96;&#41;'>&#40;&#42;&#180;&#8704;&#96;&#41;</option><option value='&#40;&#42;&#65439;&#8711;&#65439;&#41;'>&#40;&#42;&#65439;&#8711;&#65439;&#41;</option><option value='&#40;&#42;&#65439;&#12540;&#65439;&#41;'>&#40;&#42;&#65439;&#12540;&#65439;&#41;</option><option value='&#40;&#12288;&#65439;&#32;&#51;&#65439;&#41;'>&#40;&#12288;&#65439;&#32;&#51;&#65439;&#41;</option><option value='&#40;&#32;&#180;&#12540;&#96;&#41;'>&#40;&#32;&#180;&#12540;&#96;&#41;</option><option value='&#40;&#32;&#12539;&#95;&#12445;&#12539;&#41;'>&#40;&#32;&#12539;&#95;&#12445;&#12539;&#41;</option><option value='&#40;&#32;&#180;&#95;&#12445;&#96;&#41;'>&#40;&#32;&#180;&#95;&#12445;&#96;&#41;</option><option value='&#40;&#42;&#180;&#1076;&#96;&#41;'>&#40;&#42;&#180;&#1076;&#96;&#41;</option><option value='&#40;&#12539;&#12540;&#12539;&#41;'>&#40;&#12539;&#12540;&#12539;&#41;</option><option value='&#40;&#12539;&#8704;&#12539;&#41;'>&#40;&#12539;&#8704;&#12539;&#41;</option><option value='&#40;&#12445;&#8704;&#65381;&#41;'>&#40;&#12445;&#8704;&#65381;&#41;</option><option value='&#40;&#12291;&#8704;&#12291;&#41;'>&#40;&#12291;&#8704;&#12291;&#41;</option><option value='&#40;&#42;&#65439;&#8704;&#65439;&#42;&#41;'>&#40;&#42;&#65439;&#8704;&#65439;&#42;&#41;</option><option value='&#40;&#32;&#65439;&#8704;&#12290;&#41;'>&#40;&#32;&#65439;&#8704;&#12290;&#41;</option><option value='&#40;&#32;&#96;&#1076;&#180;&#41;'>&#40;&#32;&#96;&#1076;&#180;&#41;</option><option value='&#40;&#96;&#949;&#180;&#32;&#41;'>&#40;&#96;&#949;&#180;&#32;&#41;</option><option value='&#40;&#96;&#12526;&#180;&#32;&#41;'>&#40;&#96;&#12526;&#180;&#32;&#41;</option><option value='&#963;&#96;&#8704;&#180;&#41;'>&#963;&#96;&#8704;&#180;&#41;</option><option value='&#32;&#65439;&#8704;&#65439;&#41;&#963;'>&#32;&#65439;&#8704;&#65439;&#41;&#963;</option><option value='&#65439;&#32;&#8704;&#65439;&#41;&#12494;'>&#65439;&#32;&#8704;&#65439;&#41;&#12494;</option><option value='&#40;&#9580;&#65439;&#1076;&#65439;&#41;'>&#40;&#9580;&#65439;&#1076;&#65439;&#41;</option><option value='&#40;&#124;&#124;&#124;&#65439;&#1076;&#65439;&#41;'>&#40;&#124;&#124;&#124;&#65439;&#1076;&#65439;&#41;</option><option value='&#40;&#32;&#65439;&#1076;&#65439;&#41;'>&#40;&#32;&#65439;&#1076;&#65439;&#41;</option><option value='&#931;&#40;&#32;&#65439;&#1076;&#65439;&#41;'>&#931;&#40;&#32;&#65439;&#1076;&#65439;&#41;</option><option value='&#40;&#32;&#59;&#65439;&#1076;&#65439;&#41;'>&#40;&#32;&#59;&#65439;&#1076;&#65439;&#41;</option><option value='&#40;&#32;&#59;&#180;&#1076;&#96;&#41;'>&#40;&#32;&#59;&#180;&#1076;&#96;&#41;</option><option value='&#40;&#12288;&#1076;&#32;&#41;&#32;&#65439;&#32;&#65439;'>&#40;&#12288;&#1076;&#32;&#41;&#32;&#65439;&#32;&#65439;</option><option value='&#40;&#32;&#9737;&#1076;&#8857;&#41;'>&#40;&#32;&#9737;&#1076;&#8857;&#41;</option><option value='&#40;&#40;&#40;&#12288;&#65439;&#1076;&#65439;&#41;&#41;&#41;'>&#40;&#40;&#40;&#12288;&#65439;&#1076;&#65439;&#41;&#41;&#41;</option><option value='&#40;&#32;&#96;&#32;&#12539;&#180;&#41;'>&#40;&#32;&#96;&#32;&#12539;&#180;&#41;</option><option value='&#40;&#32;&#180;&#1076;&#96;&#41;'>&#40;&#32;&#180;&#1076;&#96;&#41;</option><option value='&#40;&#32;&#45;&#1076;&#45;&#41;'>&#40;&#32;&#45;&#1076;&#45;&#41;</option><option value='&#40;&#62;&#1076;&#60;&#41;'>&#40;&#38;&#103;&#116;&#59;&#1076;&#38;&#108;&#116;&#59;&#41;</option><option value='&#65381;&#65439;&#40;&#32;&#65417;&#1076;&#96;&#65439;&#41;'>&#65381;&#65439;&#40;&#32;&#65417;&#1076;&#96;&#65439;&#41;</option><option value='&#40;&#32;&#84;&#1076;&#84;&#41;'>&#40;&#32;&#84;&#1076;&#84;&#41;</option><option value='&#40;&#65507;&#8711;&#65507;&#41;'>&#40;&#65507;&#8711;&#65507;&#41;</option><option value='&#40;&#65507;&#51;&#65507;&#41;'>&#40;&#65507;&#51;&#65507;&#41;</option><option value='&#40;&#65507;&#65392;&#65507;&#41;'>&#40;&#65507;&#65392;&#65507;&#41;</option><option value='&#40;&#65507;&#32;&#46;&#32;&#65507;&#41;'>&#40;&#65507;&#32;&#46;&#32;&#65507;&#41;</option><option value='&#40;&#65507;&#30399;&#65507;&#41;'>&#40;&#65507;&#30399;&#65507;&#41;</option><option value='&#40;&#65507;&#33400;&#65507;&#41;'>&#40;&#65507;&#33400;&#65507;&#41;</option><option value='&#40;&#65507;&#65087;&#65507;&#41;'>&#40;&#65507;&#65087;&#65507;&#41;</option><option value='&#40;&#65507;&#65078;&#65507;&#41;'>&#40;&#65507;&#65078;&#65507;&#41;</option><option value='&#12542;&#40;&#180;&#969;&#65439;&#65344;&#41;'>&#12542;&#40;&#180;&#969;&#65439;&#65344;&#41;</option><option value='&#40;&#42;&#180;&#969;&#96;&#42;&#41;'>&#40;&#42;&#180;&#969;&#96;&#42;&#41;</option><option value='&#40;&#12539;&#969;&#12539;&#41;'>&#40;&#12539;&#969;&#12539;&#41;</option><option value='&#40;&#32;&#180;&#12539;&#969;&#41;'>&#40;&#32;&#180;&#12539;&#969;&#41;</option><option value='&#40;&#65344;&#12539;&#969;&#41;'>&#40;&#65344;&#12539;&#969;&#41;</option><option value='&#40;&#180;&#12539;&#969;&#12539;&#96;&#41;'>&#40;&#180;&#12539;&#969;&#12539;&#96;&#41;</option><option value='&#40;&#96;&#12539;&#969;&#12539;&#180;&#41;'>&#40;&#96;&#12539;&#969;&#12539;&#180;&#41;</option><option value='&#40;&#32;&#96;&#95;&#12387;&#180;&#41;'>&#40;&#32;&#96;&#95;&#12387;&#180;&#41;</option><option value='&#40;&#32;&#96;&#12540;&#180;&#41;'>&#40;&#32;&#96;&#12540;&#180;&#41;</option><option value='&#40;&#32;&#180;&#95;&#12387;&#96;&#41;'>&#40;&#32;&#180;&#95;&#12387;&#96;&#41;</option><option value='&#40;&#32;&#180;&#961;&#96;&#41;'>&#40;&#32;&#180;&#961;&#96;&#41;</option><option value='&#40;&#32;&#65439;&#969;&#65439;&#41;'>&#40;&#32;&#65439;&#969;&#65439;&#41;</option><option value='&#40;&#111;&#65439;&#969;&#65439;&#111;&#41;'>&#40;&#111;&#65439;&#969;&#65439;&#111;&#41;</option><option value='&#40;&#12288;&#94;&#969;&#94;&#41;'>&#40;&#12288;&#94;&#969;&#94;&#41;</option><option value='&#40;&#65377;&#9685;&#8704;&#9685;&#65377;&#41;'>&#40;&#65377;&#9685;&#8704;&#9685;&#65377;&#41;</option><option value='&#47;&#40;&#32;&#9685;&#8255;&#8255;&#9685;&#32;&#41;&#92;'>&#47;&#40;&#32;&#9685;&#8255;&#8255;&#9685;&#32;&#41;&#92;</option><option value='&#12542;&#40;&#180;&#949;&#96;&#12542;&#41;'>&#12542;&#40;&#180;&#949;&#96;&#12542;&#41;</option><option value='&#40;&#12494;&#65439;&#8704;&#65439;&#41;&#12494;'>&#40;&#12494;&#65439;&#8704;&#65439;&#41;&#12494;</option><option value='&#40;&#963;&#65439;&#1076;&#65439;&#41;&#963;'>&#40;&#963;&#65439;&#1076;&#65439;&#41;&#963;</option><option value='&#40;&#963;&#65439;&#8704;&#65439;&#41;&#963;'>&#40;&#963;&#65439;&#8704;&#65439;&#41;&#963;</option><option value='&#124;&#1076;&#65439;&#32;&#41;'>&#124;&#1076;&#65439;&#32;&#41;</option><option value='&#9475;&#38651;&#26609;&#9475;'>&#9475;&#38651;&#26609;&#9475;</option><option value='&#65439;&#40;&#12388;&#1076;&#96;&#65439;&#41;'>&#65439;&#40;&#12388;&#1076;&#96;&#65439;&#41;</option><option value='&#65439;&#8491;&#65439;&#32;&#41;&#12288;'>&#65439;&#8491;&#65439;&#32;&#41;&#12288;</option><option value='&#8834;&#24417;&#9734;&#41;&#41;&#1076;&#96;&#41;'>&#8834;&#24417;&#9734;&#41;&#41;&#1076;&#96;&#41;</option><option value='&#8834;&#24417;&#9734;&#41;&#41;&#1076;&#180;&#41;'>&#8834;&#24417;&#9734;&#41;&#41;&#1076;&#180;&#41;</option><option value='&#8834;&#24417;&#9734;&#41;&#41;&#8704;&#96;&#41;'>&#8834;&#24417;&#9734;&#41;&#41;&#8704;&#96;&#41;</option><option value='&#40;&#180;&#8704;&#40;&#40;&#9734;&#12511;&#12388;'>&#40;&#180;&#8704;&#40;&#40;&#9734;&#12511;&#12388;</option></select>"
"</td></tr>"
"<tr><td>Image:</td><td colspan='2'><input type=\"file\" name=\"input_file\" style='width:100%%'/></td></tr>"
"</table>"
"</form>";

char *html_redir = "<script type=\"text/javascript\">setTimeout(function(){window.location=\"/\";}, 2000);</script>Success!Back to the <a href=\"/\">homepage</a> in 2s.";
char *html_error = "<script type=\"text/javascript\">setTimeout(function(){window.location=\"/\";}, 2000);</script>%s, Back to the <a href=\"/\">homepage</a> in 2s.";
char *html_redirtothread = "<script type=\"text/javascript\">setTimeout(function(){window.location=\"/thread/%d\";}, 2000);</script>Success!Back to <a href=\"/thread/%d\">thread no.%d</a> in 2s.";
//char *html_emoji = 
char *html_header =
"<html>"
"<head>"
"<meta charset='UTF-8'/><meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1'/>"
"<title>%s</title>"
"<style>"
".header {background:#F0E0D6; display:table; padding: 0.2em}"
".thread {margin: 3px}"
".thread tr td {padding: 2px;word-wrap:break-word; word-break:break-all;}"
"body {background: #ffe;}"
"a {text-decoration: none}"
".pager {margin:3px; padding: 5px; background-color:white; border:solid 1px #ccc}"
".postform {border: dashed 1px;padding: 3px;max-width:400px;width:100%%; margin:auto}"
"#formtitle {background-color: #21759B;color: white; text-align:center}"
".del,.del a {color: #aaa}"
"img {"
"max-height: 250px; max-width: 250px;"
"height: expression((document.documentElement.clientHeight||document.body.clientHeight) > 250?\"250px\":\"\");"
"width: expression((document.documentElement.clientWidth||document.body.clientWidth) > 250?\"250px\":\"\");"
"}"
".quote {margin: 5px}"
"div.img {float: left; margin: 0.5em;"
"max-height: 250px; max-width: 250px;"
"height: expression((document.documentElement.clientHeight||document.body.clientHeight) > 250?\"250px\":\"\");"
"width: expression((document.documentElement.clientWidth||document.body.clientWidth) > 250?\"250px\":\"\");}"
"hr {clear:left}"
".red {color:red}"
".state {"
"background-color: deepskyblue;"
"color: white;"
"padding: 0.1em;"
"border-radius: 0.3em;"
"}"
"</style>"
"<script type='text/javascript'>"
"function askdel(n){if(confirm('Delete thread No.' + n + '?')) window.location = '/delete/' + n;}"
"</script>"
"</head>"
"<body>"
"<div style='text-align: center'><a href='/'><h1>%s</h1></a></div>";

char* imgadj =
"<script type = \"text/javascript\">"
"	var imgs = document.getElementsByClassName('imgs');"
"	for (i = 0; i < imgs.length; ++i){"
"		console.log(imgs[i].width);"
"	}"
"</script>";