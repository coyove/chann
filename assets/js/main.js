function setupListeners(){
    document.getElementById('comment').onkeydown = function(e){
        if(e.keyCode==13&&e.ctrlKey) {
            if(confirm('是否送出内容?')) document.getElementById('post-form').submit();
        }
    };
    document.getElementById("emoji").onchange = function(e){
        var em='';
        var elem=document.getElementById('comment');
        if(this.selectedIndex) em=this.options[this.selectedIndex].text;
        this.selectedIndex=0;
        if(document.selection){
            elem.focus();
            sel = document.selection.createRange();
            sel.text=em;
        }
        else if (elem.selectionStart||elem.selectionStart=='0') {
            var startPos=elem.selectionStart;
            var endPos=elem.selectionEnd;
            elem.value=elem.value.substring(0, startPos)+em+elem.value.substring(endPos,elem.value.length);
        } else {
            elem.value+=em;
            elem.focus();
        }
    };
    document.getElementById("options").onchange=function(e){
        document.getElementById('opt').value=this.value;
    };
    document.getElementById("select-file").onchange=function(e){
        var e2=document.getElementById("true-select");
        var filename=this.value.replace(/^.*[\\\/]/, '');
        e2.innerHTML='附加图片'+(filename?(': '+filename):'');
    }

}