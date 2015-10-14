#include "lang.h"

// The color template used by this site can be found here:
// http://www.colourlovers.com/web/trends/websites/7515/Wonderful_Copenhagen_-_k248benhavn_billige_hoteller_i_k248benhavn_ferie_k248benhavn_forlystelser_i_k248benhavn_hotel_i_k248benhavn_hotel_k248benhavn_hoteler_k248benhavn_hoteller_i_k248benhavn_hoteller_k248benhavn_kultur
// Template in short:
//  #FFFFFF
//  #E0EAF2
//  #E2DED7
//  #CECAC3
//  #F0EBE4

char *html_form =
"<form id='post-form' method=\"POST\" action=\"%s\" enctype=\"multipart/form-data\">"
    "<div id='input-area' class='%s' title='%s'>"
        "<div>"
            "<table><tr>"
                "<td style='width:100%%'><input type =\"text\" name=\"input_name\" style='width:100%%' value='"STRING_UNTITLED"'/></td>"
                "<td style='text-align:right'><input class='wp-btn' type =\"submit\" value='"STRING_POST_BUTTON"'/></td>"
            "</tr></table>"
        "</div>"
        "<textarea rows='8' style='width:100%%' name='input_content' placeholder='"STRING_CTRL_ENTER_POST"' id='comment'></textarea>"
        "<p>"
            "<input id='select-file' type=\"file\" name=\"input_file\" style='width:100%%;display:none'/>"
            STRING_YOU_CAN
            "<a href='javascript:void(0)' onclick='var e=document.getElementById(\"comment\");e.value += e.value ? \"\\n>>No.\" : \">>No.\";e.focus();'>"STRING_REF_A_THREAD"</a>,&nbsp;"
            "<a id='true-select' href='javascript:void(0)' onclick='document.getElementById(\"select-file\").click()'>"STRING_IMAGE"</a>,&nbsp;"
            "<select id='emoji'>"
                "<option>"STRING_INSERT_EMOJI"</option><option>|∀ﾟ</option><option>(´ﾟДﾟ`)</option><option>(;´Д`)</option><option>(｀･ω･)</option><option>(=ﾟωﾟ)=</option><option>| ω・´)</option><option>|-` )</option><option>|д` )</option><option>|ー` )</option><option>|∀` )</option><option>(つд⊂)</option><option>(ﾟДﾟ≡ﾟДﾟ)</option><option>(＾o＾)ﾉ</option><option>(|||ﾟДﾟ)</option><option>( ﾟ∀ﾟ)</option><option>( ´∀`)</option><option>(*´∀`)</option><option>(*ﾟ∇ﾟ)</option><option>(*ﾟーﾟ)</option><option>(　ﾟ 3ﾟ)</option><option>( ´ー`)</option><option>( ・_ゝ・)</option><option>( ´_ゝ`)</option><option>(*´д`)</option><option>(・ー・)</option><option>(・∀・)</option><option>(ゝ∀･)</option><option>(〃∀〃)</option><option>(*ﾟ∀ﾟ*)</option><option>( ﾟ∀。)</option><option>( `д´)</option><option>(`ε´ )</option><option>(`ヮ´ )</option><option>σ`∀´)</option><option> ﾟ∀ﾟ)σ</option><option>ﾟ ∀ﾟ)ノ</option><option>(╬ﾟдﾟ)</option><option>(|||ﾟдﾟ)</option><option>( ﾟдﾟ)</option><option>Σ( ﾟдﾟ)</option><option>( ;ﾟдﾟ)</option><option>( ;´д`)</option><option>(　д ) ﾟ ﾟ</option><option>( ☉д⊙)</option><option>(((　ﾟдﾟ)))</option><option>( ` ・´)</option><option>( ´д`)</option><option>( -д-)</option><option>(&gt;д&lt;)</option><option>･ﾟ( ﾉд`ﾟ)</option><option>( TдT)</option><option>(￣∇￣)</option><option>(￣3￣)</option><option>(￣ｰ￣)</option><option>(￣ . ￣)</option><option>(￣皿￣)</option><option>(￣艸￣)</option><option>(￣︿￣)</option><option>(￣︶￣)</option><option>ヾ(´ωﾟ｀)</option><option>(*´ω`*)</option><option>(・ω・)</option><option>( ´・ω)</option><option>(｀・ω)</option><option>(´・ω・`)</option><option>(`・ω・´)</option><option>( `_っ´)</option><option>( `ー´)</option><option>( ´_っ`)</option><option>( ´ρ`)</option><option>( ﾟωﾟ)</option><option>(oﾟωﾟo)</option><option>(　^ω^)</option><option>ヾ(´ε`ヾ)</option><option>(ノﾟ∀ﾟ)ノ</option><option>(σﾟдﾟ)σ</option><option>(σﾟ∀ﾟ)σ</option><option>|дﾟ )</option><option>┃電柱┃</option><option>ﾟ(つд`ﾟ)</option><option>ﾟÅﾟ )　</option><option>⊂彡☆))д`)</option><option>⊂彡☆))д´)</option><option>⊂彡☆))∀`)</option><option>(´∀((☆ミつ</option></select>"
            "<input type = \"text\" name = \"input_email\" class='hiding' id='opt'/>"                    
            ",&nbsp;<select id='options'>"
                "<option value=''>"STRING_OPTIONS"</option>"
                "<option value='sage'>"STRING_OPT_SAGE"</option>"
                "<option value='delete'>"STRING_OPT_DELETE"</option>"
                "<option value='url'>"STRING_OPT_URL"</option>"
            "</select>"
            "<script>"
                // "window.onkeyup=function(e){ ctrlDown=false; };"
                // "window.onkeydown=function(e){ if(e.keyCode==17) ctrlDown=true; };"
                "document.getElementById('comment').onkeydown=function(e){"
                    "if(e.keyCode==13&&e.ctrlKey) {"
                        "if(confirm('"STRING_CONFIRM_POST"')) document.getElementById('post-form').submit();"
                    "}"
                "};"
                "document.getElementById(\"emoji\").onchange=function(e){"
                    "var em='';"
                    "var elem=document.getElementById('comment');"
                    "if(this.selectedIndex) em=this.options[this.selectedIndex].text;"
                    "this.selectedIndex=0;"
                    "if(document.selection){"
                    "    elem.focus();"
                    "    sel = document.selection.createRange();"
                    "    sel.text=em;"
                    "}"
                    "else if (elem.selectionStart||elem.selectionStart=='0') {"
                    "    var startPos=elem.selectionStart;"
                    "    var endPos=elem.selectionEnd;"
                    "    elem.value=elem.value.substring(0, startPos)+em+elem.value.substring(endPos,elem.value.length);"
                    "} else {"
                    "    elem.value+=em;"
                    "    elem.docus();"
                    "}"
                "};"
                "document.getElementById(\"options\").onchange=function(e){"
                    "document.getElementById('opt').value=this.value;"
                "};"
                "document.getElementById(\"select-file\").onchange=function(e){"
                    "var e2=document.getElementById(\"true-select\");"
                    "var filename=this.value.replace(/^.*[\\\\\\/]/, '');"
                    "e2.innerHTML='"STRING_IMAGE"'+(filename?(': '+filename):'');"
                "}"
            "</script>"
        "</p>"
    "</div>"
"</form>";

char *show_hide_button =
"<button class='wp-btn' onclick='document.getElementById(\"input-area\").className=\"\"; document.getElementById(\"showdiv\").className=\"hiding\"'>"
    "&#128172;&nbsp;"STRING_NEW_THREAD
"</button>"
"<button class='wp-btn' onclick='window.location=\"/list\";'>"
    "&#128100;&nbsp;"STRING_MY_POSTS
"</button>";

char *html_error = 
"<script type=\"text/javascript\">"
    "setTimeout(function(){window.location=\"%s\";}, 2000);"
"</script>"
"%s, "STRING_AUTO_RETURN;

char *html_redirtothread = 
"<script type=\"text/javascript\">"
    "setTimeout(function(){window.location=\"/thread/%d\";}, 2000);"
"</script>"
STRING_RETURN_THREAD;
//char *html_emoji = 
char *html_header =
"<!DOCTYPE html>"
"<html>"
"<head>"
"<title>%s</title>"
"<meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"/>"
"<meta charset='UTF-8'/><meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1'/>"
"<link rel='icon' type='image/png' href='/images/favicon.png'>"
// "<!--[if IE]><!-->"
// "<link rel='stylesheet' type='text/css' href='/assets/font-ie.css'>"
// "<!--<![endif]-->"
// "<!--[if !IE]><!-->"
"<link rel='stylesheet' type='text/css' href='/assets/font.css'>"
// "<!--<![endif]-->"
"<link rel='stylesheet' type='text/css' href='/assets/main.css'>"
"<script type='text/javascript' src='/assets/main.js'></script>"
"<script type='text/javascript'>"
"var ctrlDown=false;"
"function microAjax(B,A){this.bindFunction=function(E,D){return function(){return E.apply(D,[D])}};this.stateChange=function(D){if(this.request.readyState==4){this.callbackFunction(this.request.responseText)}};this.getRequest=function(){if(window.ActiveXObject){return new ActiveXObject(\"Microsoft.XMLHTTP\")}else{if(window.XMLHttpRequest){return new XMLHttpRequest()}}return false};this.postBody=(arguments[2]||\"\");this.callbackFunction=A;this.url=B;this.request=this.getRequest();if(this.request){var C=this.request;C.onreadystatechange=this.bindFunction(this.stateChange,this);if(this.postBody!==\"\"){C.open(\"POST\",B,true);C.setRequestHeader(\"X-Requested-With\",\"XMLHttpRequest\");C.setRequestHeader(\"Content-type\",\"application/x-www-form-urlencoded\");C.setRequestHeader(\"Connection\",\"close\")}else{C.open(\"GET\",B,true)}C.send(this.postBody)}};"
"function ajst(id){"
    "if(document.getElementsByClassName){"
        "microAjax(\"/api/\"+id,function (msg){"
            "if((/(<script)/).test(msg)){window.location.href='/thread/'+id;return;}"
            "var tmp=document.getElementsByClassName('div-thread-'+id);"
            "var elems=Array.prototype.slice.call(tmp, 0);"
            "for(var i=0;i<elems.length;i++){"
                "elems[i].innerHTML=msg+'<div style=\"clear:left\"/>';"
                "elems[i].className='header ref';"
            "}"
            "cvtm(false);"
        "});"
    "}"
    "else{"
        "window.location.href='/thread/'+id;"
    "}"
"};"
"function admc(id){"
    "microAjax(\"/adminconsole/\" + id, function (msg){"
        "var elem=document.getElementById('admin-'+id);"
        "elem.innerHTML=msg;"
        "var n=elem.parentNode.nextSibling;"
        "n.innerText=n.innerHTML;"
    "});"
"};"
"function qref(id){"
    "var e=document.getElementById('comment');"
    "e.value+=e.value?('\\n>>No.'+id+'\\n'):('>>No.'+id+'\\n');"
    "e.focus();"
"};"
"function cvtm(change){"
"};"
"function enim(id,lurl,surl){"
    "var elem=document.getElementById(id);"
    "var p=elem.parentNode;"
    "var c=elem.childNodes[0];"
    "if(c.className=='img-s'||c.className=='img-n'){"
        "p.className='enlarge-'+c.className.split('-')[1];"
        "c.className='img-l';"
        "if(lurl) c.src=lurl;"
        "return;"
    "}"
    "if(c.className=='img-l'){"
        "c.className='img-'+p.className.split('-')[1];"
        "p.className='img';"
        "if(surl) c.src=surl;"
    "}"
"}"
"function exim(id,url){"
    "var elem=document.getElementById(id);"
    "var p=elem.parentNode;"
    "p.innerHTML='<div class=img><a href=javascript:void(0) id='+id+' onclick=enim(\"'+id+'\")><img class=img-s src=/images/'+url+'/></a></div>';"
"}"
"function isar(){"
    "if(!document.getElementById('input-area')) document.getElementById('start-new-thread').className='hiding';"
    "var windowHeight = window.innerHeight;"
    "var divHeight = document.getElementById('container').clientHeight;"
    "if(divHeight<windowHeight)"
        "document.getElementById('page-footer').className='page-footer-fixed';"
"}"
"</script>"
"</head>"
"<body onload='isar()'>"
"<div id='container'>"
"<div class='page-header'><div class='page-header-lim'>"
    "<p style='float:left'><a href='/' style='vertical-align:middle'><img id='header-image' src='/images/main.png'></img></a></p>"
    "<p style='float:right;margin:1em;'>"
        "<span onclick='document.getElementById(\"input-area\").className=\"\";' id='start-new-thread'>"
            "&#128172;&nbsp;"STRING_NEW_THREAD
        "</span>&nbsp;"
        "<script>"
            "if((/(thread|daerht|list)/).test(window.location.href)) document.getElementById('start-new-thread').className='hiding';"
        "</script>"
        "<span onclick='window.location.href=\"/list\";'>"
            "&#128100;&nbsp;"STRING_MY_POSTS
        "</span>"
        "<span onclick='window.location.href=\"/gallery/1\";'>"
            "&#128247;&nbsp;"STRING_GALLERY_PAGE
        "</span>"
    "</p>"
    "<br style='clear:both'>"
"</div></div>"
"<div class='wrapper'>";

char *html_footer =
"</div>"
"<div class='page-footer' id='page-footer'>"
"<p style='color:#aaa;margin:0;float:left'>%s</p>"
"<p style='color:#aaa;margin:0;float:right'><small>%ld</small></p>"
"<br style='clear:both'>"
"</div>"
"</div>"
"</body>"
"</html>";