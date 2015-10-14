<!--[if info_page]-->
<script type="text/javascript">
    setTimeout(function(){window.location="/";}, 2000);
</script>
{{CONTENT}}, 2s后返回<a href="/">主页</a>.
<!--[endif]-->

<!--[if return_page]-->
<script type="text/javascript">
    setTimeout(function(){window.location="/thread/{{THREAD_NO}}";}, 2000);
</script>
回复成功, 2s后返回<a href="/thread/{{THREAD_NO}}">No.{{THREAD_NO}}</a>.
<!--[endif]-->