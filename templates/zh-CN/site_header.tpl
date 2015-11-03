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

function microAjax(B,A){this.bindFunction=function(E,D){return function(){return E.apply(D,[D])}};this.stateChange=function(D){if(this.request.readyState==4){this.callbackFunction(this.request.responseText)}};this.getRequest=function(){if(window.ActiveXObject){return new ActiveXObject("Microsoft.XMLHTTP")}else{if(window.XMLHttpRequest){return new XMLHttpRequest()}}return false};this.postBody=(arguments[2]||"");this.callbackFunction=A;this.url=B;this.request=this.getRequest();if(this.request){var C=this.request;C.onreadystatechange=this.bindFunction(this.stateChange,this);if(this.postBody!==""){C.open("POST",B,true);C.setRequestHeader("X-Requested-With","XMLHttpRequest");C.setRequestHeader("Content-type","application/x-www-form-urlencoded");C.setRequestHeader("Connection","close")}else{C.open("GET",B,true)}C.send(this.postBody)}};

function is_mobile() {
  var check = false;
  (function(a){if(/(android|bb\d+|meego).+mobile|avantgo|bada\/|blackberry|blazer|compal|elaine|fennec|hiptop|iemobile|ip(hone|od)|iris|kindle|lge |maemo|midp|mmp|mobile.+firefox|netfront|opera m(ob|in)i|palm( os)?|phone|p(ixi|re)\/|plucker|pocket|psp|series(4|6)0|symbian|treo|up\.(browser|link)|vodafone|wap|windows ce|xda|xiino/i.test(a)||/1207|6310|6590|3gso|4thp|50[1-6]i|770s|802s|a wa|abac|ac(er|oo|s\-)|ai(ko|rn)|al(av|ca|co)|amoi|an(ex|ny|yw)|aptu|ar(ch|go)|as(te|us)|attw|au(di|\-m|r |s )|avan|be(ck|ll|nq)|bi(lb|rd)|bl(ac|az)|br(e|v)w|bumb|bw\-(n|u)|c55\/|capi|ccwa|cdm\-|cell|chtm|cldc|cmd\-|co(mp|nd)|craw|da(it|ll|ng)|dbte|dc\-s|devi|dica|dmob|do(c|p)o|ds(12|\-d)|el(49|ai)|em(l2|ul)|er(ic|k0)|esl8|ez([4-7]0|os|wa|ze)|fetc|fly(\-|_)|g1 u|g560|gene|gf\-5|g\-mo|go(\.w|od)|gr(ad|un)|haie|hcit|hd\-(m|p|t)|hei\-|hi(pt|ta)|hp( i|ip)|hs\-c|ht(c(\-| |_|a|g|p|s|t)|tp)|hu(aw|tc)|i\-(20|go|ma)|i230|iac( |\-|\/)|ibro|idea|ig01|ikom|im1k|inno|ipaq|iris|ja(t|v)a|jbro|jemu|jigs|kddi|keji|kgt( |\/)|klon|kpt |kwc\-|kyo(c|k)|le(no|xi)|lg( g|\/(k|l|u)|50|54|\-[a-w])|libw|lynx|m1\-w|m3ga|m50\/|ma(te|ui|xo)|mc(01|21|ca)|m\-cr|me(rc|ri)|mi(o8|oa|ts)|mmef|mo(01|02|bi|de|do|t(\-| |o|v)|zz)|mt(50|p1|v )|mwbp|mywa|n10[0-2]|n20[2-3]|n30(0|2)|n50(0|2|5)|n7(0(0|1)|10)|ne((c|m)\-|on|tf|wf|wg|wt)|nok(6|i)|nzph|o2im|op(ti|wv)|oran|owg1|p800|pan(a|d|t)|pdxg|pg(13|\-([1-8]|c))|phil|pire|pl(ay|uc)|pn\-2|po(ck|rt|se)|prox|psio|pt\-g|qa\-a|qc(07|12|21|32|60|\-[2-7]|i\-)|qtek|r380|r600|raks|rim9|ro(ve|zo)|s55\/|sa(ge|ma|mm|ms|ny|va)|sc(01|h\-|oo|p\-)|sdk\/|se(c(\-|0|1)|47|mc|nd|ri)|sgh\-|shar|sie(\-|m)|sk\-0|sl(45|id)|sm(al|ar|b3|it|t5)|so(ft|ny)|sp(01|h\-|v\-|v )|sy(01|mb)|t2(18|50)|t6(00|10|18)|ta(gt|lk)|tcl\-|tdg\-|tel(i|m)|tim\-|t\-mo|to(pl|sh)|ts(70|m\-|m3|m5)|tx\-9|up(\.b|g1|si)|utst|v400|v750|veri|vi(rg|te)|vk(40|5[0-3]|\-v)|vm40|voda|vulc|vx(52|53|60|61|70|80|81|83|85|98)|w3c(\-| )|webc|whit|wi(g |nc|nw)|wmlb|wonu|x700|yas\-|your|zeto|zte\-/i.test(a.substr(0,4)))check = true})(navigator.userAgent||navigator.vendor||window.opera);
  return check;
}
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

function admc(id, p){
    var e =document.getElementById('admin-' + id);
    e.style.display="block";

    e.onmouseout = function(evt){
        e.style.display="none";
    }
};

function qref(id){
    if(!is_mobile()){
        var e=document.getElementById('post-form');
        e.style.position = "fixed";
        e.style.right = 0;
        e.style.bottom=0;
        e.style.zIndex = '99';

        var e1 = document.getElementById("options");
        var p = e1.parentNode;
        var a = document.createElement('a');
        a.innerHTML="[ 退出小窗 ]";
        a.setAttribute("href", "#");
        a.setAttribute("onclick", "exitwin()");
        p.insertBefore(a, e1.nextSibling);

        document.getElementById("input-area").className="";
        footeradj();
    }

    e=document.getElementById('comment');
    e.value+=e.value?('\n>>No.'+id+'\n'):('>>No.'+id+'\n');
    e.focus();
};

function exitwin(){
    document.getElementById('post-form').style.position='relative';
    var e=document.getElementById("options");
    var p=e.parentNode;
    p.removeChild(e.nextSibling);
}

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
    footeradj();
    window.onresize = footeradj();

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

function footeradj(){
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
                <p style='float:left'><a href='/' style='vertical-align:middle'><img id='header-image' src='/assets/images/main.png'></img></a></p>
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