<!DOCTYPE html>
<html>
<head>

<title>
    {{SITE_TITLE}}
    <!--[if my_post_page]-->
    &nbsp;- 我的记录
    <!--[endif]-->

    <!--[if success_post_page]-->
    &nbsp;- 送出成功
    <!--[endif]-->

    <!--[if upload_image_page]-->
    &nbsp;- 上传图片成功
    <!--[endif]-->

    <!--[if timeline_page]-->
    &nbsp;- 第{{CURRENT_PAGE}}页
    <!--[endif]-->

    <!--[if gallery_page]-->
    &nbsp;相册 - 第{{CURRENT_PAGE}}页
    <!--[endif]-->

    <!--[if thread_page]-->
    &nbsp;- No.{{THREAD_NO}} {{THREAD_TITLE}}
    <!--[endif]-->

    <!--[if admin_list_all_page]-->
    &nbsp;- 全站列表
    <!--[endif]-->

    <!--[if admin_panel_page]-->
    &nbsp;- 控制面板
    <!--[endif]-->

    <!--[if archive_mode]-->
    的存档&nbsp;- 第{{CURRENT_PAGE}}页
    <!--[endif]-->
</title>

<meta http-equiv="X-UA-Compatible" content="IE=edge"/>
<meta charset='UTF-8'/><meta name='viewport' content='width=device-width,initial-scale=1,maximum-scale=1'/>
<link rel='icon' type='image/png' href='/images/favicon.png'>
<link rel='stylesheet' type='text/css' href='/assets/font.css'>
<link rel='stylesheet' type='text/css' href='/assets/main.css'>
<script type='text/javascript' src='/assets/main.js'></script>
<script type='text/javascript'>

function microAjax(B,A){this.bindFunction=function(E,D){return function(){return E.apply(D,[D])}};this.stateChange=function(D){if(this.request.readyState==4){this.callbackFunction(this.request.responseText)}};this.getRequest=function(){if(window.ActiveXObject){return new ActiveXObject("Microsoft.XMLHTTP")}else{if(window.XMLHttpRequest){return new XMLHttpRequest()}}return false};this.postBody=(arguments[2]||"");this.callbackFunction=A;this.url=B;this.request=this.getRequest();if(this.request){var C=this.request;C.onreadystatechange=this.bindFunction(this.stateChange,this);if(this.postBody!==""){C.open("POST",B,true);C.setRequestHeader("X-Requested-With","XMLHttpRequest");C.setRequestHeader("Content-type","application/x-www-form-urlencoded");C.setRequestHeader("Connection","close")}else{C.open("GET",B,true)}C.send(this.postBody)}};

function ajst(id){
    if(document.getElementsByClassName){
        microAjax("/api/"+id,function (msg){
            if((/(<script)/).test(msg)){window.location.href='/thread/'+id;return;}
            var tmp=document.getElementsByClassName('div-thread-'+id);
            var elems=Array.prototype.slice.call(tmp, 0);
            for(var i=0;i<elems.length;i++){
                elems[i].innerHTML=msg+'<div style="clear:left"/>';
                elems[i].className='header ref';
            }
            
        });
    }
    else{
        window.location.href='/thread/'+id;
    }
};

function admc(id){
    microAjax("/adminconsole/" + id, function (msg){
        var elem=document.getElementById('admin-'+id);
        elem.innerHTML=msg;
        var n=elem.parentNode.nextSibling;
        n.innerText=n.innerHTML;
    });
};

function qref(id){
    var e=document.getElementById('comment');
    e.value+=e.value?('\n>>No.'+id+'\n'):('>>No.'+id+'\n');
    e.focus();
};

function enim(id,lurl,surl){
    var elem=document.getElementById(id);
    var p=elem.parentNode;
    var c=elem.childNodes[0];
    if(c.className=='img-s'||c.className=='img-n'){
        p.className='enlarge-'+c.className.split('-')[1];
        c.className='img-l';
        if(lurl) c.src=lurl;
        return;
    }
    if(c.className=='img-l'){
        c.className='img-'+p.className.split('-')[1];
        p.className='img';
        if(surl) c.src=surl;
    }
};

function exim(id,url){
    var elem=document.getElementById(id);
    var p=elem.parentNode;
    p.innerHTML='<div class=img><a href=javascript:void(0) id='+id+' onclick=enim("'+id+'")><img class=img-s src=/images/'+url+'/></a></div>';
};

function isar(){
    if(!document.getElementById('input-area')) document.getElementById('start-new-thread').className='hiding';
    var windowHeight = window.innerHeight;
    var divHeight = document.getElementById('container').clientHeight;
    if(divHeight<windowHeight)
        document.getElementById('page-footer').className='page-footer-fixed';
}
</script>
</head>
<body onload='isar()'>
    <div id='container'>
        <div class='page-header'>
            <div class='page-header-lim'>
                <p style='float:left'><a href='/' style='vertical-align:middle'><img id='header-image' src='/images/main.png'></img></a></p>
                <p style='float:right;margin:1em;'>
                    <span onclick='document.getElementById("input-area").className="";' id='start-new-thread'>
                        &#128172;&nbsp;发表新串
                    </span>&nbsp;
                    <script>
                        if((/(thread|daerht|list)/).test(window.location.href)) document.getElementById('start-new-thread').className='hiding';
                    </script>
                    <span onclick='window.location.href="/list";'>
                        &#128100;&nbsp;我的记录
                    </span>
                    <span onclick='window.location.href="/gallery/1";'>
                        &#128247;&nbsp;相册
                    </span>
                </p>
                <br style='clear:both'>
            </div>
        </div>
        <div class='wrapper'>