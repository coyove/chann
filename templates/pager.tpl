<hr>
<div style='text-align:center;'>
<!--[if timeline_page]-->
    <a class='pager pager-arrow' href="/page/{{PREVIOUS_PAGE}}">&#171;</a>
        <!--[loop before_pages]-->
            <!--[if before_pages=1]--><a class='pager' href="/">1</a><!--[endif]-->
            <!--[if !before_pages=1]-->
                <a class='pager' href="/page/{{before_pages}}">{{before_pages}}</a>
            <!--[endif]-->
        <!--[endloop]-->

        <a class='pager-inv'>{{CURRENT_PAGE}}</a>

        <!--[loop after_pages]-->
            <!--[if after_pages=1]--><a class='pager' href="/">1</a><!--[endif]-->
            <!--[if !after_pages=1]-->
                <a class='pager' href="/page/{{after_pages}}">{{after_pages}}</a>
            <!--[endif]-->
        <!--[endloop]-->
    <a class='pager pager-arrow' href="/page/{{NEXT_PAGE}}">&#187;</a>
<!--[endif]-->

<!--[if gallery_page]-->
    <a class='pager pager-arrow' href="/gallery/{{PREVIOUS_PAGE}}">&#171;</a>
        <!--[loop before_pages]-->
            <a class='pager' href="/gallery/{{before_pages}}">{{before_pages}}</a>
        <!--[endloop]-->

        <a class='pager-inv'>{{CURRENT_PAGE}}</a>

        <!--[loop after_pages]-->
            <a class='pager' href="/gallery/{{after_pages}}">{{after_pages}}</a>
        <!--[endloop]-->
    <a class='pager pager-arrow' href="/gallery/{{NEXT_PAGE}}">&#187;</a>
<!--[endif]-->
</div>

<br style='clear:both'>