using Electro;
using System;

class Player : Entity
{
    private TransformComponent mTransformComponent;
    private SpriteRendererComponent mSpriteRenderer;
    private float mSpeed = 2;

    public void OnStart()
    {
        string name = GetComponent<TagComponent>().Tag;
        Electro.Console.LogInfo("Starting Entity with name: " + name);
        mTransformComponent = GetComponent<TransformComponent>();
        mSpriteRenderer = GetComponent<SpriteRendererComponent>();
    }

    public void OnUpdate(float ts)
    {
        Vector3 trans = mTransformComponent.Translation;
        Vector3 rot = mTransformComponent.Rotation;

        var rand = new Random();
        mSpriteRenderer.SetColor(new Vector4((float)rand.NextDouble(), (float)rand.NextDouble(), (float)rand.NextDouble(), (float)rand.NextDouble()));

        if (Input.IsKeyPressed(KeyCode.A))
            trans.X -= mSpeed * ts;
        if(Input.IsKeyPressed(KeyCode.D))
            trans.X += mSpeed * ts;
        if (Input.IsKeyPressed(KeyCode.W))
            trans.Y += mSpeed * ts;
        if (Input.IsKeyPressed(KeyCode.S))
            trans.Y -= mSpeed * ts;
        if(Input.IsKeyPressed(KeyCode.Q))
            rot.Z += mSpeed * ts;
        if (Input.IsKeyPressed(KeyCode.E))
            rot.Z -= mSpeed * ts;

        mTransformComponent.Translation = trans;
        mTransformComponent.Rotation = rot;
    }
}