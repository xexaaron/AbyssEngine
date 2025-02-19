# Resources

Resources are objects like Shaders, Fonts, and Textures that are represented
by a resource type and a handle or key, into a map that contains the actual
object. 

This documentation will be referring to the actual object and the type/handle structure 
interchangeably.


## Creation

Resources each have their own create method as shown below.
To create a resource you need the application rendering context.

```cpp
Resource font    = Font::create(ctx, ...)
Resource shader  = Shader::create(ctx, ...)
Resource texture = Texture::create(ctx, ...)
// etc...
```

## Acessing resource data

To access the resources underlying data you have to get the ResourceClass from
the applications rendering context.

Usually this is done through an overridden method in a derived Object class
so for this example we will assume that as our scope.

Resource are loaded on a seperate thread called the LoadThread.

Resource::Handle objects are ensured to be incrementing values starting from zero.
therefore we can return a resource type and handle before the underlying
data has been loaded.  

The code looks the exact same for the texture as it does
any other resource.

```cpp

class MyObject : public Object {
public:
    MyObject(App*);
    void on_create(App*, bool) override;
private:
    Resource m_Texture;
}

MyObject::MyObject(App* app) {
    Ref<Context> ctx = app->ctx();
    m_Texture = Texture::create(ctx, ...);
    // Do not attempt to retrieve the texture
    // it may not have been loaded yet.
    // the texture is guaranteed to be loaded
    // by the time MyObject::on_create is called.
}

MyObject::on_create(App* app, bool) {
    Ref<Context> ctx = app->ctx();
    ResoureClass<Texture>& textures = ctx->textures();
    Ref<Texture> texture = textures.at(m_Texture);
}
```





