using Spike;

class CameraController : Entity
{
    private TransformComponent m_Transform;
    private TransformComponent m_PlayerTransform;
    public void Start()
    {
        Console.LogDebug("Camera Started");
        Console.LogInfo(null);
    }

    public void Update(float ts)
    {
        m_Transform = GetComponent<TransformComponent>();
        m_PlayerTransform = FindEntityByTag("Player").GetComponent<TransformComponent>();

        m_Transform.Translation = new Vector3(m_PlayerTransform.Translation.X, m_PlayerTransform.Translation.Y, 21.25f);
    }

    public void FixedUpdate(float fixedTimestep)
    {
    }
}
