<!--[if post_new_thread]-->
<form id='post-form' method="POST" action="/post_thread" enctype="multipart/form-data">
    <div id='input-area' class='hiding' title='发表新串'>
<!--[endif]-->
<!--[if reply_to_thread]-->
<form id='post-form' method="POST" action="/post_reply/{{THREAD_NO}}" enctype="multipart/form-data">
    <div id='input-area' class='postform' title='回复串No.{{THREAD_NO}}'>
<!--[endif]-->
        <div>
            <table>
            <tr>
                <td style='width:100%'><input type ="text" name="input_name" style='width:100%' value='无标题'/></td>
                <td style='text-align:right'><input class='wp-btn' type ="submit" value='送出'/></td>
            </tr>
            </table>
        </div>
        <textarea rows='8' style='width:100%' name="input_content" placeholder='Ctrl+Enter送出内容' id='comment'></textarea>
        <p>
            <input id='select-file' type="file" name="input_file" style='width:100%;display:none'/>
            你可以:&nbsp;
            <a href='javascript:void(0)' 
                onclick='var e=document.getElementById("comment");e.value+=e.value?"\n>>No.":">>No.";e.focus();'>
                引用串
            </a>,&nbsp;
            <a id='true-select' href='javascript:void(0)' onclick='document.getElementById("select-file").click()'>
                附加图片
            </a>,&nbsp;
            <select id='emoji'>
                <option>插入颜文字</option><option>|∀ﾟ</option><option>(´ﾟДﾟ`)</option><option>(;´Д`)</option><option>(｀･ω･)</option><option>(=ﾟωﾟ)=</option><option>| ω・´)</option><option>|-` )</option><option>|д` )</option><option>|ー` )</option><option>|∀` )</option><option>(つд⊂)</option><option>(ﾟДﾟ≡ﾟДﾟ)</option><option>(＾o＾)ﾉ</option><option>(|||ﾟДﾟ)</option><option>( ﾟ∀ﾟ)</option><option>( ´∀`)</option><option>(*´∀`)</option><option>(*ﾟ∇ﾟ)</option><option>(*ﾟーﾟ)</option><option>(　ﾟ 3ﾟ)</option><option>( ´ー`)</option><option>( ・_ゝ・)</option><option>( ´_ゝ`)</option><option>(*´д`)</option><option>(・ー・)</option><option>(・∀・)</option><option>(ゝ∀･)</option><option>(〃∀〃)</option><option>(*ﾟ∀ﾟ*)</option><option>( ﾟ∀。)</option><option>( `д´)</option><option>(`ε´ )</option><option>(`ヮ´ )</option><option>σ`∀´)</option><option> ﾟ∀ﾟ)σ</option><option>ﾟ ∀ﾟ)ノ</option><option>(╬ﾟдﾟ)</option><option>(|||ﾟдﾟ)</option><option>( ﾟдﾟ)</option><option>Σ( ﾟдﾟ)</option><option>( ;ﾟдﾟ)</option><option>( ;´д`)</option><option>(　д ) ﾟ ﾟ</option><option>( ☉д⊙)</option><option>(((　ﾟдﾟ)))</option><option>( ` ・´)</option><option>( ´д`)</option><option>( -д-)</option><option>(&gt;д&lt;)</option><option>･ﾟ( ﾉд`ﾟ)</option><option>( TдT)</option><option>(￣∇￣)</option><option>(￣3￣)</option><option>(￣ｰ￣)</option><option>(￣ . ￣)</option><option>(￣皿￣)</option><option>(￣艸￣)</option><option>(￣︿￣)</option><option>(￣︶￣)</option><option>ヾ(´ωﾟ｀)</option><option>(*´ω`*)</option><option>(・ω・)</option><option>( ´・ω)</option><option>(｀・ω)</option><option>(´・ω・`)</option><option>(`・ω・´)</option><option>( `_っ´)</option><option>( `ー´)</option><option>( ´_っ`)</option><option>( ´ρ`)</option><option>( ﾟωﾟ)</option><option>(oﾟωﾟo)</option><option>(　^ω^)</option><option>ヾ(´ε`ヾ)</option><option>(ノﾟ∀ﾟ)ノ</option><option>(σﾟдﾟ)σ</option><option>(σﾟ∀ﾟ)σ</option><option>|дﾟ )</option><option>┃電柱┃</option><option>ﾟ(つд`ﾟ)</option><option>ﾟÅﾟ )　</option><option>⊂彡☆))д`)</option><option>⊂彡☆))д´)</option><option>⊂彡☆))∀`)</option><option>(´∀((☆ミつ</option></select>
            <input type="text" name="input_email" class='hiding' id='opt'/>
            ,&nbsp;
            <select id='options'>
                <option value=''>附加选项</option>
                <option value='sage'>SAGE</option>
                <option value='delete'>Delete</option>
                <option value='url'>URL</option>
                <!--[if is_admin]-->
                <option value='update'>编辑</option>
                <option value='update-html'>HTML编辑</option>
                <!--[endif]-->
            </select>
            
            <script type="text/javascript">
                setupListeners();
            </script>
        </p>
    </div>
</form>