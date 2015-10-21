<!--[if info_page]-->
<script type="text/javascript">
    setTimeout(function(){window.location="/";}, 2000);
</script>
{{CONTENT}}, 2s后返回<a href="/">主页</a>.
<!--[endif]-->

<!--[if return_page]-->
	<!--[if THREAD_NO=0]-->
	<script type="text/javascript">
	    setTimeout(function(){window.location="/";}, 2000);
	</script>
	成功发表新串, 2s后返回<a href="/">主页</a>.
	<!--[endif]-->
	<!--[if !THREAD_NO=0]-->
	<script type="text/javascript">
	    setTimeout(function(){window.location="/thread/{{THREAD_NO}}";}, 2000);
	</script>
	回复成功, 2s后返回<a href="/thread/{{THREAD_NO}}">No.{{THREAD_NO}}</a>.
	<!--[endif]-->
<!--[endif]-->

<!--[if admin_page]-->
<script type="text/javascript">
    setTimeout(function(){window.location="/admin";}, 2000);
</script>
登录成功 ({{COOKIE}}), 2s后返回<a href="/admin">控制面板</a>.
<!--[endif]-->

<!--[if upload_image_page]-->
已上传图像:
<div style='background-color:white; padding: 1em;border: dashed 1px'>
    <a href="/images/{{IMAGE}}"><img src="/images/{{IMAGE}}"></a>
</div>
<!--[endif]-->