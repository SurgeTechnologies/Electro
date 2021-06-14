using Electro;

class Camera : Entity
{
    private Entity mPlayer;

    public void OnStart()
    {
        mPlayer = FindEntityByTag("Player");
    }

    public void OnUpdate(float ts)
    {
        Translation = mPlayer.GetComponent<TransformComponent>().Translation + new Vector3(0.0f, 0.5f, -20.0f);
    }
}