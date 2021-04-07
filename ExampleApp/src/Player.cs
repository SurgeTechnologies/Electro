using Electro;

class Player : Entity
{
    private RigidBodyComponent mRigidBody;
    public float mSpeed = 15.0f;
    public void OnStart()
    {
        mRigidBody = GetComponent<RigidBodyComponent>();

        AddCollisionBeginCallback(OnPlayerCollisionBegin);
        AddCollisionEndCallback(OnPlayerCollisionEnd);
        AddTriggerBeginCallback(OnPlayerTriggerBegin);
        AddTriggerEndCallback(OnPlayerTriggerEnd);
    }
    void OnPlayerCollisionBegin(float value)
    {
        Console.LogDebug("I have started colliding!");
    }
    void OnPlayerCollisionEnd(float value)
    {
        Console.LogDebug("I have ended colliding!");
    }
    void OnPlayerTriggerBegin(float value)
    {
        Console.LogDebug("Trigger started!");
    }
    void OnPlayerTriggerEnd(float value)
    {
        Console.LogDebug("Trigger ended!");
    }

    public void OnUpdate(float ts)
    {
        if (Input.IsKeyPressed(KeyCode.D))
            mRigidBody.AddForce(new Vector3(mSpeed, 0.0f, 0.0f), ForceMode.Acceleration);
        if (Input.IsKeyPressed(KeyCode.A))
            mRigidBody.AddForce(new Vector3(-mSpeed, 0.0f, 0.0f), ForceMode.Acceleration);
        if (Input.IsKeyPressed(KeyCode.W))
            mRigidBody.AddForce(new Vector3(0.0f, 0.0f, -mSpeed), ForceMode.Acceleration);
        if (Input.IsKeyPressed(KeyCode.S))
            mRigidBody.AddForce(new Vector3(10.0f, 0.0f, mSpeed), ForceMode.Acceleration);

        if(Input.IsKeyPressed(KeyCode.Space))
            mRigidBody.AddForce(new Vector3(0.0f, 1.0f, 0.0f), ForceMode.Impulse);
    }
}
