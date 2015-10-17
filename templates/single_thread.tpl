<div>
    <!--a reply has a left margin-->
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
            <!--[if show_admin]-->
                <ul class="container">
                    <li class="dropdown">
                        [ <a href="javascript:void(0)" data-toggle="dropdown">管理</a> ]&nbsp;
                        <ul class="dropdown-menu">
                            <li class="splitter">No.{{THREAD_NO}}</li>
                            <li><a href="/admin#state:{{THREAD_STATE}}:{{THREAD_NO}}">编辑状态</a></li>
                            <li class="splitter">{{THREAD_IP}}</li>
                            <li><a href="/admin#banip:{{THREAD_IP}}">查询IP地址、封锁</a></li>
                            <li><a href="/list/ip/{{THREAD_IP}}">该IP发言记录</a></li>
                            <li class="splitter">{{THREAD_POSTER}}</li>
                            <li><a href="/admin#banid:{{THREAD_POSTER}}">封锁{{THREAD_POSTER}}</a></li>
                            <li><a href="/list/{{THREAD_POSTER}}">查看{{THREAD_POSTER}}的发言</a></li>
                        </ul>
                    </li>
                </ul>
            <!--[endif]-->
            <!--[if !show_admin]-->
            <!--[if is_sameone]-->
                <ul class="container">
                    <li class="dropdown">
                        [ <a href="javascript:void(0)" data-toggle="dropdown">管理</a> ]&nbsp;
                        <ul class="dropdown-menu">
                            <li class="splitter">No.{{THREAD_NO}}</li>
                            <li><a href="/del/{{THREAD_NO}}">自主删除</a></li>
                        </ul>
                    </li>
                </ul>
            <!--[endif]-->
            <!--[endif]-->
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
                </ssid> 发布于&nbsp;
                <!--[if show_easy_date]-->
                    <!--[if THREAD_POST_DATE=0]-->今天<!--[endif]-->
                    <!--[if THREAD_POST_DATE=1]-->昨天<!--[endif]-->
                    <!--[if THREAD_POST_DATE=2]-->前天<!--[endif]-->
                <!--[endif]-->
                <!--[if !show_easy_date]-->{{THREAD_POST_DATE}}<!--[endif]-->
                &nbsp;{{THREAD_POST_TIME}}   
            </span>&nbsp;

            <!--[if show_reply]-->
            [ <a href="/thread/{{THREAD_NO}}">
                <!--[if archive]-->查看<!--[endif]-->
                <!--[if !archive]-->回复<!--[endif]-->
            </a> ]
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
                </ssid> 发布于&nbsp;
                <!--[if show_easy_date]-->
                    <!--[if THREAD_POST_DATE=0]-->今天<!--[endif]-->
                    <!--[if THREAD_POST_DATE=1]-->昨天<!--[endif]-->
                    <!--[if THREAD_POST_DATE=2]-->前天<!--[endif]-->
                <!--[endif]-->
                <!--[if !show_easy_date]-->{{THREAD_POST_DATE}}<!--[endif]-->
                &nbsp;{{THREAD_POST_TIME}}
            </span>&nbsp;
        </div>
        <div class='alert-box'>Access Denied</div>
        <!--[endif]-->
    </div>
</div>