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
<link rel='icon' type='image/png' href='/assets/favicon.png'>

<link rel='stylesheet' type='text/css' href='/assets/css/font.css'>
<link rel='stylesheet' type='text/css' href='/assets/css/main.css'>
<link rel='stylesheet' type='text/css' href='/assets/css/admin.css'>

<script type='text/javascript' src='/assets/js/main.js'></script>
<script type='text/javascript'>

function ajst(id){
    if(document.getElementsByClassName){
        Helper.ajax("/api/"+id,function (msg){
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

function init(){
    if(!Helper.$('input-area')) 
        Helper.$('start-new-thread').className='hiding';

    window.onresize = function(){
        var windowHeight = window.innerHeight;
        var divHeight = Helper.$('container').clientHeight;
        if(divHeight<windowHeight)
            Helper.$('page-footer').className='page-footer-fixed';
    };

    window.onresize();

    var dropdown = document.querySelectorAll('.dropdown');
    var dropdownArray = Array.prototype.slice.call(dropdown, 0);
    dropdownArray.forEach(function (el) {
        var button = el.querySelector('a[data-toggle="dropdown"]'), menu = el.querySelector('.dropdown-menu'), arrow = button.querySelector('i.icon-arrow');
        button.onclick = function (event) {
            if (!menu.hasClass('show')) {
                menu.classList.add('show');
                menu.classList.remove('hide');
                arrow.classList.add('open');
                arrow.classList.remove('close');
                event.preventDefault();
            } else {
                menu.classList.remove('show');
                menu.classList.add('hide');
                arrow.classList.remove('open');
                arrow.classList.add('close');
                event.preventDefault();
            }
        };
    });
    Element.prototype.hasClass = function (className) {
        return this.className && new RegExp('(^|\\s)' + className + '(\\s|$)').test(this.className);
    };
}
</script>
</head>
<body onload='init()'>
    <div id='container'>
        <div class='page-header'>
            <div class='page-header-lim'>
                <p style='float:left'><a href='{{HOMEPAGE}}' style='vertical-align:middle'><img id='header-image' src='/assets/images/main.png'></img></a></p>
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
                    <span onclick='window.location.href="/admin";'>
                        &#128272;&nbsp;管理员
                    </span>
                </p>
                <br style='clear:both'>
            </div>
        </div>
        <div class='wrapper'>