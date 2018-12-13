AsyncTask名称来源于Android.AsyncTask,但更加灵活。
(不要问我为什么这么多类名都和Android中的一样,我表示很无奈呀,android框架中各组件的取名太贴切了，我想不出比这更好的名字了)
AsyncTask可用来提交比较耗时的任务到工作线程，执行之前和执行之后会通知AsyncTask,可以避免阻塞当前线程。

AsyncTask比较适合用来执行可能持续短暂时间(比如最多几秒钟)的任务,如果可能持续较长时间，建议自行创建Looper和TaskLooper.

Android.AsyncTask只能由main UI looper发起
CoreLooper.AsyncTask没有此限制，可以在任意Looper中创建和发起AsyncTask


