var ws;// = new WebSocket('wss://waifu.cc/ws');

if(window.location.href=='https://waifu.cc/thread/271')
	ws = new WebSocket('wss://waifu.cc/ws');	
else{
}

var rooms = [];
var myID;
var hist = [];
var emoji = ["|∀ﾟ", "(´ﾟДﾟ`)", "(;´Д`)", "(｀･ω･)", "(=ﾟωﾟ)=", "| ω・´)", "|-` )", "|д` )", "|ー` )", "|∀` )", "(つд⊂)", "(ﾟДﾟ≡ﾟДﾟ)", "(＾o＾)ﾉ", "(|||ﾟДﾟ)", "( ﾟ∀ﾟ)", "( ´∀`)", "(*´∀`)", "(*ﾟ∇ﾟ)", "(*ﾟーﾟ)", "(　ﾟ 3ﾟ)", "( ´ー`)", "( ・_ゝ・)", "( ´_ゝ`)", "(*´д`)", "(・ー・)", "(・∀・)", "(ゝ∀･)", "(〃∀〃)", "(*ﾟ∀ﾟ*)", "( ﾟ∀。)", "( `д´)", "(`ε´ )", "(`ヮ´ )", "σ`∀´)", " ﾟ∀ﾟ)σ", "ﾟ ∀ﾟ)ノ", "(╬ﾟдﾟ)", "(|||ﾟдﾟ)", "( ﾟдﾟ)", "Σ( ﾟдﾟ)", "( ;ﾟдﾟ)", "( ;´д`)", "(　д ) ﾟ ﾟ", "( ☉д⊙)", "(((　ﾟдﾟ)))", "( ` ・´)", "( ´д`)", "( -д-)", "(&gt;д&lt;)", "･ﾟ( ﾉд`ﾟ)", "( TдT)", "(￣∇￣)", "(￣3￣)", "(￣ｰ￣)", "(￣ . ￣)", "(￣皿￣)", "(￣艸￣)", "(￣︿￣)", "(￣︶￣)", "ヾ(´ωﾟ｀)", "(*´ω`*)", "(・ω・)", "( ´・ω)", "(｀・ω)", "(´・ω・`)", "(`・ω・´)", "( `_っ´)", "( `ー´)", "( ´_っ`)", "( ´ρ`)", "( ﾟωﾟ)", "(oﾟωﾟo)", "(　^ω^)", "(｡◕∀◕｡)", "/( ◕‿‿◕ )\\", "ヾ(´ε`ヾ)", "(ノﾟ∀ﾟ)ノ", "(σﾟдﾟ)σ", "(σﾟ∀ﾟ)σ", "|дﾟ )", "┃電柱┃", "ﾟ(つд`ﾟ)", "ﾟÅﾟ )　", "⊂彡☆))д`)", "⊂彡☆))д´)", "⊂彡☆))∀`)", "(´∀((☆ミつ"];
var winFocus = true;
var ori = document.title;

function pushDiv(postTime, message, poster, returnDOM){
	var div = document.createElement('div');
	var d = new Date(parseInt(postTime) * 1000);
  	var hr = d.getHours();
  	var mn = d.getMinutes();
  	var sec = d.getSeconds();
  	var html;

  	html = '<tr><td>';
  	html += '<small>' + (d.getMonth() + 1) + "/" + d.getDate() + '</small> ';
  	html += (hr < 10 ? '0' + hr:hr) + ":" + (mn < 10 ? '0' +mn:mn) + ":" + (sec< 10 ? '0' + sec:sec);
  	html += '</td>';

  	var msg = message;
  	if((/>>No\.(\d+)/).test(msg)){
  		var tno = msg.match(/>>No\.(\d+)/)[1];
  		msg = '<div class="div-thread-'+tno+'"><a href="javascript:ajst('+tno+')">&gt;&gt;No.'+tno+'</a></div>';
  	} else if((/(http|https|ftp):\/\/(.+)/).test(msg)){
  		msg = '<a href="' + msg + '" target="_blank">' + msg + '</a><br>';
  	}
  	else{
  		msg = msg.replace(/[<>\{\}\[\]\\]/gi, '');
  		msg += '<br>';
  	}

  	switch(poster){
  		case "Admin":
  			break;
  	 	case myID:
	  		html += ("<td class='aright'>" + msg + '</td></tr>');
	  		break;
	  	default:
	  		html += "<td class='aleft'>";
	  		html += msg;
	  		html += '<other>' + poster + '</other>';
	  		html += '</td></tr>';		
  	}

  	div.innerHTML = '<table class="self">' + html + '</table>';

  	if(returnDOM) return div;

  	var mel = document.getElementById('messages');
  	mel.appendChild(div);
	mel.scrollTop = mel.scrollHeight;
}

function pushDiv2(message, returnDOM){
	var div = document.createElement('div');
	var d = new Date();
  	var hr = d.getHours();
  	var mn = d.getMinutes();
  	var sec = d.getSeconds();
  	var html = (hr < 10 ? '0' + hr:hr) + ":" + (mn < 10 ? '0' +mn:mn) + ":" + (sec< 10 ? '0' + sec:sec);
  	html += ("&nbsp;&nbsp;" + message);
 
  	div.innerHTML = '<div class="system-msg">' + html + '</div>';

  	if(returnDOM) return div;

  	var mel = document.getElementById('messages');
  	mel.appendChild(div);
	mel.scrollTop = mel.scrollHeight;
}

function alertNewMessage(){
	
	// var flag = false;

	// function blinkTitle(){
	// 	flag = !flag;
	// 	document.title = (flag ? "(新消息)" : "") + ori;
	// 	if(!winFocus) 
	// 		setInterval(blinkTitle, 1000);
	// 	else
	// 		document.title = ori;
	// }

	if(!winFocus) {
		document.title = "(新消息)" + ori;//setInterval(blinkTitle, 1000);
		var audio = new Audio('/alert.mp3');
		audio.play();
	}
}
// ws.onopen = function(ev)  { pushDiv2('Websocket 打开'); };

ws.onerror = function(ev) { pushDiv2('Websocket 错误'); };
ws.onclose = function(ev) { pushDiv2('Websocket 已经关闭'); };

ws.onmessage = function(ev) {
	var m = (ev.data || '').match(/^(\S+) (.+)/);

	switch (m[1]){
		case 'id':
			myID = m[2];
			ws.send('join 0');
			pushDiv2('进入聊天室');
			break;
		case 'msg':
		  	var mm = m[2].match(/(\S+) (\S+) (\S+) (.+)/);

		  	if(mm[2] == 'Admin')
		  		pushDiv2("管理员：" + mm[4]);
		  	else
		  		pushDiv(mm[3], mm[4], mm[2]);

		  	alertNewMessage();

		  	break;
		case 'history':
	  		var mm = m[2].match(/(\S+) (\S+) (.+)/);

	  		if(mm[2] == 'Admin')
	  			hist.push(pushDiv2("管理员：" + mm[3], true));
	  		else
	  			hist.push(pushDiv(mm[1], mm[3], mm[2], true));
	  		break;
		case 'end':
			var mel = document.getElementById('messages');
			for(var i = hist.length - 1; i >=0; i--)
				mel.appendChild(hist[i]);
		
			pushDiv2('以上为历史消息');

			if(myID == '(null)')
				pushDiv2('没有饼干');
			else
				pushDiv2('登录饼干：' + myID);
	}
};

window.onload = function() {
	// document.getElementById('send_button').style.display = 'none';

	var elem = document.getElementById('send_input');

	elem.onkeyup = function(ev) {
		if(ev.keyCode == 13) {
			var e = document.getElementById('send_input');
			ws.send('msg ' + e.value);
			e.value = '';
		}
	};

	var emojiselect = document.createElement('select');
	var html = [];
	html.push('<option value="">颜文字</option>');
	for(var i = 0; i < emoji.length; ++i)
		html.push('<option value="' + emoji[i] + '">' + emoji[i] + '</option>');
	emojiselect.innerHTML = html.join('');
	emojiselect.onchange = function(ev){ elem.value += this.value; }

	elem.parentNode.insertBefore(emojiselect, elem.nextSibling);

	window.onfocus = function () { 
		winFocus = true; 
		document.title = ori;
	}
	window.onblur = function () { winFocus = false; }

	if(cvtm) cvtm(false);
};