<div>
    <!--[a reply has a left margin]-->
    <!--[if reply]-->
    <div class='holder'>
        <holder></holder>
    </div>
    <!--[endif]-->

    <div class="thread<!--[if reply]--> header<!--[endif]-->">
        <!--[if normal_display]-->
        <!--[if image_attached]-->
            <!--[if show_size_only]-->
                <div class='img'>
                    <a id='img-{{THREAD_NO}}' href='javascript:void(0)' onclick='exim("img-{{THREAD_NO}}","{{THREAD_IMAGE}}")'>
                        [查看图片 ({{THREAD_IMAGE_SIZE}} kb)]
                    </a>
                </div>
            <!--[endif]-->
            <!--[if show_full_image]-->
                <div class='img'>
                    <a id='img-{{THREAD_NO}}' href='javascript:void(0)' 
                        onclick="enim('img-{{THREAD_NO}}','/images/{{THREAD_IMAGE}}','{{THREAD_THUMB_PREFIX}}{{THREAD_IMAGE}}')">
                        <img class='img-<!--[if reply]-->s<!--[endif]--><!--[if !reply]-->n<!--[endif]-->' src='{{THREAD_THUMB_PREFIX}}{{THREAD_IMAGE}}'/>
                    </a>
                </div>
            <!--[endif]-->
        <!--[endif]-->
        <!--[if file_attached]-->
            <div class='img file'>
                <a class='wp-btn' href='/images/{{THREAD_IMAGE}}'>打开文件</a>
            </div>
        <!--[endif]-->

        <div class="reply-header">

            <!--[if show_reply]-->
            <a href="javascript:qref({{THREAD_NO}})">No.{{THREAD_NO}}</a>&nbsp;
            <!--[endif]--><!--[if !show_reply]-->
            <a href='/thread/{{THREAD_NO}}'>No.{{THREAD_NO}}</a>&nbsp;
            <!--[endif]-->

            <ttt>{{THREAD_TITLE}}</ttt>&nbsp;
            <span class="tmsc">
                <ssid>
                    <!--[if thread_poster_is_admin]--><red>Admin</red><!--[endif]-->
                    <!--[if !thread_poster_is_admin]-->{{THREAD_POSTER}}<!--[endif]-->

                    <!--[if thread_poster_is_sameone]--><pox>PO</pox><!--[endif]-->
                </ssid> 发布于 {{THREAD_POST_TIME}}
            </span>&nbsp;

            <!--[if show_reply]-->
            [ <a href="/thread/{{THREAD_NO}}">
                <!--[if archive]-->查看<!--[endif]-->
                <!--[if !archive]-->回复<!--[endif]-->
            </a> ]
            <!--[endif]-->

            <!--[if show_admin]-->
            &nbsp;
            <span id='admin-{{THREAD_NO}}'>
                <a class='wp-btn' onclick='javascript:admc({{THREAD_NO}})'>{{THREAD_IP}}</a>
            </span>
            <!--[endif]-->
        </div>
        <div class="quote">
            {{THREAD_CONTENT}}
        </div>

        <!--[if sage]--><red><b>&#128078;&nbsp;该串已被SAGE</b></red><br/><!--[endif]-->
        <!--[if lock]--><red><b>&#128274;&nbsp;该串已被锁定</b></red><br/><!--[endif]-->
        <!--[if delete]--><red><b>&#10006;&nbsp;该串已被删除</b></red><br/><!--[endif]-->

        <!--[if show_num_replies]-->
        <a class='dcyan' href='/thread/{{THREAD_NO}}'>&#128172;&nbsp;{{NUM_REPLIES}} 条回复</a><br/>
        <!--[endif]-->

        <!--[endif]-->

        <!--[if !normal_display]-->
        <div class='reply-header'>
            No.{{THREAD_NO}}&nbsp;<ttt>{{THREAD_TITLE}}</ttt>&nbsp;
            <span class="tmsc">
                <ssid>
                    <!--[if thread_poster_is_admin]--><red>Admin</red><!--[endif]-->
                    <!--[if !thread_poster_is_admin]-->{{THREAD_POSTER}}<!--[endif]-->
                    <!--[if thread_poster_is_sameone]--><pox>PO</pox><!--[endif]-->
                </ssid> 发布于 {{THREAD_POST_TIME}}
            </span>&nbsp;
        </div>
        <div class='alert-box'>Access Denied</div>
        <!--[endif]-->
    </div>
</div>