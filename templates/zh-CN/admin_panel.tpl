<script type="text/javascript" src="/assets/js/jquery-1.9.1.min.js"></script>

<!--[if is_admin]-->

<div class='admin-panel'>
    <table style="margin:auto" id="main-table">
    <tr><td>系統状态</td><td colspan="2">
        <div style="text-align:left">
            Memory: {{MEMORY_USAGE}} kb, Uptime: {{RUNNING_TIME}} hours
        </div>
    </td></tr>

    <!--[if is_assist]-->
    <tr><td>协助身份</td><td colspan="2">
        <div style="text-align:left">{{ASSIST_NAME}}</div>
    </td></tr>
    <!--[endif]-->

    <tr><td>修改串状态</td><td colspan=2>
        <div class=admin-warning-panel style="min-width:300px;max-width:300px;width:300px">
            <div class='alert-box alert-box-inverse' style="margin-bottom:0.5em;margin-top:0;">请小心执行</div>
            <canvas id="myCanvas" width="300" height="150"></canvas>
            <div style="text-align:center; margin-bottom: 6px; line-height:1em">
                串号No.&nbsp;<input type="text" name="new-state" value="" id="thread-no" style="width:60px"/>&nbsp;
                <button onclick="setState()">设置状态</button>
            </div>
        </div>
            <input type="text" value="" id="new-state-value" style="display:none"/>
    </td></tr>

    <tr><td>解封IP</td>
        <td><span id="display-ip-ban-list"></span><div style="display:none" id="ip-ban-list">{{IP_BAN_LIST}}</div></td>
        <td><button onclick="postData('ban', $('#select-ip-ban-list').val(), 'ip')">解封</button></td>
    </tr>

    <tr><td>解封ID</td>
        <td><span id="display-id-ban-list"></span><div style="display:none" id="id-ban-list">{{ID_BAN_LIST}}</div></td>
        <td><button onclick="postData('ban', $('#select-id-ban-list').val(), 'id')">解封</button></td>
    </tr>

    <tr><td>封IP/ID</td><td>
        <select id='select-ban-type'>
            <option value='ip'>IP</option>
            <option value='id'>ID</option>
        </select>&nbsp;
        <input id="text-ban" type="text" value=""/>
        &nbsp;<button onclick="findIP()">?</button>
        </td>
        <td><button onclick="postData('ban', $('#text-ban').val(), $('#selec-ban-type').val())">封锁</button></td>
    </tr>

    <tr><td colspan="3" id="ip-result" style="display:none">
        <iframe src="" id='ip-loc-display' style="width: 100%;border: 1px solid #ccc;height: 300px;"></iframe>
    </td></tr> 

    <tr><td>删除IP/ID的所有发言</td><td colspan=2>
        <div class=admin-warning-panel style="min-width:300px;max-width:300px;width:300px">
            <div class='alert-box alert-box-inverse' style="margin-bottom:0.5em;margin-top:0;">请小心执行</div>
            <div style="text-align:center; margin-bottom: 6px; line-height:1em;padding:0 0.2em">
                <select id='kill-type'>
                    <option value='ip'>IP</option>
                    <option value='id'>ID</option>
                </select>&nbsp;
                <input type="text" id="kill-all" value=""/>&nbsp;
                <button onclick="postData('kill-all', $('#kill-all').val(), $('#kill-type').val())">删除全部</button>
            </div>
        </div>
    </td></tr>

    <tr><td>搜索</td>
        <td style="line-height:2em">
            内容: <input id="text-search" type="text" value=""/><br>
            限制: <input id="text-search-limit" type="text" value="100"/>
        </td>
        <td><button onclick="postData('search', $('#text-search').val(), $('#text-search-limit').val())">搜索</button></td>
    </tr>
    <tr><td colspan="3" id="search-result" style="display:none;text-align:left"></td></tr> 

    <tr><td>退出</td>
        <!--[if is_assist]-->
            <td colspan=2><button onclick="postData('quit-assist');">退出协助管理员权限</button></td>
        <!--[endif]-->
        <!--[if !is_assist]-->
            <td colspan=2><button onclick="postData('quit-admin');">退出管理员权限</button></td>
        <!--[endif]-->
    </tr>

    </table>

    <div id="global-configs" style="display:none">{{GLOBAL_CONFIGS}}</div>

</div>

<script type="text/javascript">
<!--[if !is_assist]-->
var gc = JSON.parse($('#global-configs').html());
var table = document.getElementById("main-table");
table.insertRow().innerHTML = "<td colspan=3><hr></td>";

for(var i in gc){
    var k = i.replace("::", "--");
    var html = "";
    if(typeof gc[i] == "boolean"){
        html = ("<td>" + i + "</td><td><select style='width:100%' id='" + k + "'>" +
            "<option " + (gc[i] ? "selected=selected":"") + " value='on'>on</option>" +
            "<option " + (gc[i] ? "":"selected=selected") + " value='off'>off</option>" +
            "</select></td>");
    }
    else{
        html = ("<tr><td>" + i + "</td><td><input type='text' id='" + k + "' value='" + gc[i] + "' style='width:100%'/></td>");
    }

    html += ("<td><button onclick=postData2('" + k + "')>确定</button></td>");

    var r = table.insertRow();
    r.innerHTML = html;
};
<!--[endif]-->

    function setState(){
        if($('#thread-no').val()){
            if(graphBits.getDel()){
                if(confirm("是否要删除No." + $('#thread-no').val()))
                    postData('delete-thread', $('#thread-no').val());    
            }else{
                postData('new-state', $('#thread-no').val(), graphBits.getValue());
            }
        }else{
            alert('请输入串号');
        }
    };

    function findIP(){
        $('#ip-result').show();
        $('#ip-loc-display').attr('src', 'https://ipinfo.io/'+$('#text-ban').val());
    }

    function postData(action, param1, param2){
        var _d = "action_name=" + action;
        if(param1) _d += "&action_1=" + param1;
        if(param2) _d += "&action_2=" + param2;

        $.ajax({
            type: "POST",
            url: "/admin_action",
            data: _d,
            success: function(msg) {
                if(msg[0] == '['){
                    $("#search-result").show();
                    console.log(msg);
                    var j =  JSON.parse(msg);
                    var html = [];
                    for(var i in j){
                        if(j[i])
                        html.push("<div class='div-thread-"+j[i]+"'><a href='javascript:ajst("+j[i]+")'>No."+j[i]+"</a></div>");
                    }
                    $("#search-result").html(html.join(''));
                }else if(msg.indexOf("quitted") > 0)
                    location.reload();
                else{
                    alert(msg);
                }
            }
        });
    }

    function postData2(configName){
        var _d = "action_name=update&action_1=" + configName.replace("--", "::") + "&action_2=" + $('#' + configName).val();

        $.ajax({
            type: "POST",
            url: "/admin_action",
            data: _d,
            success: function(msg) {
                    alert(msg);
            }
        });
    }
</script>

<script type="text/javascript">
var graphBits = (function(){
    var pos = {};
    var bits = [];
    var texts = ["锁定","未使用","未使用","SAGE","回复上限","主串","回复","正常显示"];
    var colors = ["#000","#000","#000","#000","#000","#000","#000","#000"]; 
    var display_value;
    var del_mark = false;
    var tick = new Image();
    tick.src = "/assets/images/tick.png";

    function drawCircle(ctx, x, y, r, clr){
        ctx.beginPath();
        ctx.arc(x, y, r, 0, 2 * Math.PI);
        ctx.strokeStyle = '#000000';
        ctx.lineWidth = 2;
        ctx.stroke();
        ctx.fillStyle = clr;
        ctx.fill();
    };

    function toBits(n){
        var thread_state = n;
        var cur_index = 0;

        bits = [];
        
        while(thread_state > 0){
            var mod = thread_state % 2;
            bits.push(mod);
            thread_state = parseInt(thread_state / 2);
            cur_index++;
        }
        for(cur_index; cur_index < 8; ++ cur_index) bits.push(0);

        drawBits();
    };

    function drawBits(){
        var c = document.getElementById("myCanvas");
        var ctx = c.getContext("2d");

        ctx.fillStyle = "#fff";
        ctx.fillRect(0, 0, 300, 150);

        var baseX = -30, baseY = 40;

        ctx.font = "12px Arial";
        ctx.fillStyle = "#000";
        ctx.fillText("On", 260, 34 + baseY);
        ctx.fillText("Off", 260, 64 + baseY);

        var lastPX = 0, lastPY = 0;

        for(var i = 7; i >= 0; --i){
            j = 7 - i;

            var clr = (i == 6 || i == 5) ? '#ccc' : '#000';

            pos[i] = {x: j * 30 + 50 + baseX, y: 30 + baseY};

            if(bits[i]){
                drawCircle(ctx, pos[i].x, 30 + baseY, 8, clr);
                drawCircle(ctx, pos[i].x, 60 + baseY, 8, '#fff');

                if(i != 7){
                    ctx.beginPath();
                    ctx.moveTo(lastPX, lastPY);
                    ctx.bezierCurveTo(pos[i].x, lastPY, lastPX, 30 + baseY, pos[i].x, 30 + baseY);
                    ctx.stroke();
                }

                lastPY = 30 + baseY;
                colors[j] = '#000';
            }
            else{
                drawCircle(ctx, pos[i].x, 30 + baseY, 8, '#fff');
                drawCircle(ctx, pos[i].x, 60 + baseY, 8, clr);

                if(i != 7){
                    ctx.beginPath();
                    ctx.moveTo(lastPX, lastPY);
                    ctx.bezierCurveTo(pos[i].x, lastPY, lastPX, 60 + baseY, pos[i].x, 60 + baseY);
                    ctx.stroke();
                }

                lastPY = 60 + baseY;

                colors[j] = '#aaa';
            }

            lastPX = j * 30 + 50 + baseX;
        }

        for(var i = 7; i >= 0; --i){
            j = 7 - i;

            ctx.save();
            ctx.translate(pos[i].x, baseY + 5);
            ctx.rotate(-Math.PI / 4);

            ctx.textAlign = 'left';
            ctx.fillStyle = colors[j];
            ctx.fillText(texts[j], 0, 6);

            ctx.restore();
        }

        drawCircle(ctx, pos[7].x, pos[7].y + 60, 8, del_mark ? '#f88': '#fff');
        if(del_mark) ctx.drawImage(tick, 0, 0, 18, 18, pos[7].x - 8, pos[7].y + 52, 16, 16);
        ctx.save();
        ctx.translate(pos[7].x + 15, pos[7].y + 58);
        ctx.textAlign = 'left';
        ctx.fillStyle = '#000';
        ctx.fillText('删除', 0, 6);
        ctx.restore();

        ctx.beginPath();
        ctx.lineWidth = 1;
        ctx.strokeStyle = '#aaa';
        ctx.moveTo(pos[2].x, pos[2].y + 38);
        ctx.bezierCurveTo(pos[2].x, pos[2].y + 48, pos[2].x + 15, pos[2].y + 38, pos[2].x + 15, pos[2].y + 48);
        ctx.moveTo(pos[1].x, pos[1].y + 38);
        ctx.bezierCurveTo(pos[1].x, pos[1].y + 48, pos[1].x - 15, pos[1].y + 38, pos[1].x - 15, pos[1].y + 48);
        ctx.moveTo(pos[1].x, pos[1].y - 8);
        ctx.bezierCurveTo(pos[1].x, pos[1].y - 18, pos[2].x, pos[2].y -18, pos[2].x, pos[2].y - 8);
        ctx.moveTo(pos[2].x, pos[2].y + 8); ctx.lineTo(pos[2].x, pos[2].y + 22);
        ctx.moveTo(pos[1].x, pos[1].y + 8); ctx.lineTo(pos[1].x, pos[1].y + 22);
        ctx.stroke();

        ctx.save();
        ctx.translate(pos[2].x + 15, pos[1].y + 58);
        ctx.textAlign = 'center';
        ctx.fillStyle = '#aaa';
        ctx.fillText('"主串"和"回复"的状态只能存在一个', 0, 6);
        ctx.restore();
    };

    function getValue(){
        var ret = 0;
        for(var i in bits) ret += bits[i] * Math.pow(2, i);

        return ret;
    };

    function mouseupHandler(evt) {
        var rect = document.getElementById("myCanvas").getBoundingClientRect();
        
        var downX = evt.clientX - rect.left;
        var downY = evt.clientY - rect.top;

        for(var i in pos)
            if(downX > pos[i].x - 8 && downX < pos[i].x + 8 && downY > pos[i].y - 8 && downY < pos[i].y + 38){
                bits[i] = Math.abs(bits[i] - 1);

                if((i == 1 || i == 2) && bits[1] + bits[2] != 1){
                    if(i == 1) bits[2] = Math.abs(bits[1] - 1);
                    if(i == 2) bits[1] = Math.abs(bits[2] - 1);
                }

                drawBits();
                if(display_value) display_value.value = getValue();
                break;
            }else if(downX > pos[7].x - 8 && downX < pos[7].x + 8 && downY > pos[i].y + 38){
                del_mark = !del_mark;
                drawBits();
                break;
            }

    }

    return {
        readBits: function(n, elem){
            toBits(n);
            document.getElementById("myCanvas").removeEventListener('mouseup', mouseupHandler);
            document.getElementById("myCanvas").addEventListener('mouseup', mouseupHandler, false);

            display_value = elem;
        },

        getValue: getValue,
        getDel: function(){
            return del_mark;
        }
    };
})();

</script> 

<script type="text/javascript">
    var param = window.location.href.split("#");

    graphBits.readBits(5);

    if(param.length > 1){
        var arr = param[1].split(":");
        switch(arr[0]){
            case "state":
                var thread_state = arr[1] < 0 ? parseInt(arr[1]) + 256 : arr[1];
                graphBits.readBits(thread_state);
                document.getElementById('new-state-value').value = thread_state;
                document.getElementById('thread-no').value = arr[2];
                break;
            case "banip":
                document.getElementById('text-ban').value = arr[1];
                document.getElementById('select-ban-type').value = 'ip';
                break;
            case "banid":
                document.getElementById('text-ban').value = arr[1];
                document.getElementById('select-ban-type').value = 'id';
                break;
            default:
        }
    }

    var tmp = []; tmp.push("<select id='select-ip-ban-list'><option>已封IP</option>");
    var arr = document.getElementById("ip-ban-list").innerHTML.split(",");
    for(var i in arr) if(arr[i]) tmp.push("<option value='" + arr[i] + "'>" + arr[i] + "</option>");
    tmp.push("</select>");

    document.getElementById("display-ip-ban-list").innerHTML = tmp.join('');

    var tmp = []; tmp.push("<select id='select-id-ban-list'><option>已封ID</option>");
    var arr = document.getElementById("id-ban-list").innerHTML.split(",");
    for(var i in arr) if(arr[i]) tmp.push("<option value='" + arr[i] + "'>" + arr[i] + "</option>");
    tmp.push("</select>");

    document.getElementById("display-id-ban-list").innerHTML = tmp.join('');

</script>
<!--[endif]-->

<!--[if !is_admin]-->
<div id="login-panel">
    <!-- 管理密码： -->
    <div id="login-panel-upper"><img id="login-logo" src="/assets/images/logo.png"></div>
    <div id="login-panel-lower">
        <table>
            <tr><td>&#128100;</td><td><input type="text" id="username" value="" placeholder="用户名"/></td></tr>
            <tr><td>&#128272;</td><td><input type="password" id="password" value="" placeholder="密码"/></td></tr>
        </table>
        <button onclick="login()" class="wp-btn">登录</button>
    </div>
</div>

<script type="text/javascript">
    $("#password").keyup(function(e){
        if(e.keyCode == 13) login();
    });
    
    function login(){
        $.ajax({
            type:'POST',
            url:'/admin_action',
            data:'action_name=login&action_1='+$('#username').val() + '&action_2=' + $('#password').val(),
            success: function(msg) { 
                location.reload();
            }
        });
    }
</script>
<!--[endif]-->