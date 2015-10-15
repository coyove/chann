<!--[if is_admin]-->
<div>
	<form class="admin-panel" method="POST" action="/admin_action" enctype="multipart/form-data">
		<input type="radio" name="cookie" value="on" <!--[if cookie]-->checked="checked"<!--[endif]-->/>打開Cookie<br>
		<input type="radio" name="cookie" value="off" <!--[if !cookie]-->checked="checked"<!--[endif]-->/>關閉Cookie<br>
		<input type="submit" value="確定"/>
	</form>

	<form class="admin-panel" method="POST" action="/admin_action" enctype="multipart/form-data">
		<input type="radio" name="archive" value="on" <!--[if archive]-->checked="checked"<!--[endif]-->/>全站衹讀<br>
		<input type="radio" name="archive" value="off" <!--[if !archive]-->checked="checked"<!--[endif]-->/>全站正常發串<br>
		<input type="submit" value="確定"/>
	</form>

	<form class="admin-panel" method="POST" action="/admin_action" enctype="multipart/form-data">
		用戶最大可瀏覽頁面數：
		<input type="text" name="max-page-viewable" value="{{MAX_PAGES_VIEWABLE}}"/><br>
		<input type="submit" value="確定"/>
	</form>

	<form class="admin-panel" method="POST" action="/admin_action" enctype="multipart/form-data">
		<input type="text" name="quit-admin" value="" style="display:none"/>
		<input type="submit" value="退出管理員權限"/>
	</form>
</div>
<!--[endif]-->

<!--[if !is_admin]-->
<div>
	<form class="admin-panel" method="POST" action="/admin_action" enctype="multipart/form-data">
		管理密碼：
		<input type="text" name="password" value=""/><br>
		<input type="submit" value="確定"/>
	</form>
</div>
<!--[endif]-->