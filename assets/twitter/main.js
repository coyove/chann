var cur_twi_page = 0;

Date.prototype.Format = function (fmt) { //author: meizz 
    var o = {
        "M+": this.getMonth() + 1, //月份 
        "d+": this.getDate(), //日 
        "h+": this.getHours(), //小时 
        "m+": this.getMinutes(), //分 
        "s+": this.getSeconds(), //秒 
        "q+": Math.floor((this.getMonth() + 3) / 3), //季度 
        "S": this.getMilliseconds() //毫秒 
    };
    if (/(y+)/.test(fmt)) fmt = fmt.replace(RegExp.$1, (this.getFullYear() + "").substr(4 - RegExp.$1.length));
    for (var k in o)
    if (new RegExp("(" + k + ")").test(fmt)) fmt = fmt.replace(RegExp.$1, (RegExp.$1.length == 1) ? (o[k]) : (("00" + o[k]).substr(("" + o[k]).length)));
    return fmt;
}

if(window.location.href.indexOf("/thread/355") > 0 ) getNewData();

function getNewData(new_page){
	if(new_page || new_page == 0) cur_twi_page = new_page;

	microAjax('/twitter/' + (cur_twi_page * 20), function (msg){
		var jsonData = JSON.parse(msg);
		// console.log(jsonData);
		var tl = document.getElementById('twitter-timeline');
		tl.innerHTML = "<div id='total-tweets-count'></div>";

		var elems = [];

		for(var i = 0; i < jsonData.length; ++i){
			var header = [];
			header.push('<div class="holder">&nbsp;&nbsp;</div><div class="thread header">');
			// if(jsonData[i].retweeted) console.log(jsonData[i]);
			if(jsonData[i].media){
				var iid = 'img-' + jsonData[i].id;
				var iurl = '/twi-images/' + jsonData[i].media;

				header.push('<div class=img><a id=' + iid + ' href="javascript:void(0)" ' +
							'onclick=enim("' + iid + '","' + iurl + '","' + iurl + '")>' +
							'<img class="img-s" src="' + iurl + '">' +
							'</a></div>');
			}

			header.push('<div class="reply-header">');
			var d = new Date(jsonData[i].date);

			var twi_url = "https://twitter.com/" + jsonData[i].user + "/status/" + jsonData[i].id;

			header.push('<a href="' + twi_url + '" target="_blank">No.' + jsonData[i].id + '</a>');
			header.push('&nbsp;<ttt></ttt>&nbsp;');
			header.push('<span class="tmsc"><ssid>' + jsonData[i].user + '</ssid>');
			if(jsonData[i].retweeted)
				header.push(' 转推于 ' + d.Format("yyyy-MM-dd hh:mm:ss") + '</span>&nbsp;</div>');
			else
				header.push(' 发布于 ' + d.Format("yyyy-MM-dd hh:mm:ss") + '</span>&nbsp;</div>');

			var finaltext = jsonData[i].text.replace(/((http|https)(\S+))/g, "<a href='$1' target='_blank'>$1</a>");
			header.push('<div class="quote">' + finaltext + '</div></div>');

			var e = document.createElement('div');
			e.innerHTML = header.join('');

			tl.appendChild(e);
		}

		var e = document.createElement('div');
		e.setAttribute('style', 'text-align:center');
		var html = "";

		for(var i = cur_twi_page - 1; i > cur_twi_page - 5 && i >= 0; --i){
			html = "<a class='pager' href='javascript:getNewData(" + i + ")'>" + (i + 1) + "</a>" + html;
		}

		html += "<a class='pager-inv'>" + (cur_twi_page + 1) + "</a>";

		for(var i = cur_twi_page + 1; i < cur_twi_page + 5; ++i){
			html += "<a class='pager' href='javascript:getNewData(" + i + ")'>" + (i + 1) + "</a>";
		}

		html += '<br style="clear:both">';

		e.innerHTML = html;

		tl.appendChild(e);

		microAjax("/twitter/state", function(msg){
			var j = JSON.parse(msg)[0];
			var d = new Date(j.Update_time);
			var t = new Date();
			document.getElementById('total-tweets-count').innerHTML = 
				"一共存档了 " + j.Rows + " 篇推文，上一次存档时间：" + parseInt((t-d)/1000) + " 秒之前";
		});

		window.scrollTo(0,0);
	});
	
}