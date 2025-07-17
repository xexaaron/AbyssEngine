# Object

The object class serves as a way to interact with the
app through specific events.

## Lifecycle

1. Application method `#!cpp run()` is called. Resource loading is synchronized.

2. The objects `#!cpp on_create` method is then called allowing access of resources.

3. A  `#!cpp WindowResizeEvent` is dispatched out to them triggering the objects `#!cpp on_event` method.

4. ImGui is initialized.

5. The run loop starts and every tick the objects `#!cpp on_tick` method is called.

6. Before the application is shutdown each objects `#!cpp on_destroy` method is called.

!!! Warning
    Do not attempt to use resources before `#!cpp Object::on_create` is called.

## Event Processing

When the application recieves an event it is dispatched to every object. Each object can then choose to 'subscribe' to certain events.

Each event specific function returns a boolean indicating wether to propogate the event to other objects or stop there. Returning false indicates that it should propogate.

```cpp title="Event Processing" linenums="1"

void MyObject::on_event(App* app, Event& event) {
    EventDispatcher dsp(event);
    dsp.bind(this, &MyObject::on_window_resize); // `Subscribe` to an event
    dsp.bind(this, &MyObject::on_key_typed);
}

bool MyObject::on_window_resize(WindowResizeEvent& event) {
    return false; // Propogate event
}

bool MyObject::on_key_typed(KeyTypedEvent& event) {
    return true; // Do not progate event
}

```
