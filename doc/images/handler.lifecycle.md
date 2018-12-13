图片生成方法:
https://mermaidjs.github.io/mermaid-live-editor
graph TD
A("Begin") -->|"采用make_shared< Handler>()构造Handler" |Start(Handler构造函数)
Start -->|Handler.Create或者parent.AddChild,绑定到Looper| B(Handler::OnCreate)
B -->|Destroy| C(Handler::OnDestroy)
C -->|"最后一个强引用消失时"| D(Handler 析构函数 )
D --> End(End)

