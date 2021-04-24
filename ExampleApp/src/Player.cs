using Electro;

class Player : Entity
{
    private RigidBodyComponent mRigidBody;
    private TransformComponent mCameraTransform;
    private TransformComponent mTransform;
    private TagComponent mTag;
    public float mSpeed = 15.0f;
    public void OnStart()
    {
        mTransform = GetComponent<TransformComponent>();
        mRigidBody = GetComponent<RigidBodyComponent>();
        mTag = GetComponent<TagComponent>();
        mCameraTransform = FindEntityByTag("Camera").GetComponent<TransformComponent>();
        AddCollisionBeginCallback(OnPlayerCollisionBegin);
        AddCollisionEndCallback(OnPlayerCollisionEnd);
        AddTriggerBeginCallback(OnPlayerTriggerBegin);
        AddTriggerEndCallback(OnPlayerTriggerEnd);
        Console.LogWarn("Entity named " + mTag.Tag +" has started!");
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
            mRigidBody.AddForce(new Vector3(0.0f, 0.0f, mSpeed), ForceMode.Acceleration);

        if(Input.IsKeyPressed(KeyCode.Space))
            mRigidBody.AddForce(new Vector3(0.0f, 1.0f, 0.0f), ForceMode.Impulse);

        mCameraTransform.Translation = mTransform.Translation + new Vector3(0.0f, 0.5f, -20.0f);
        RaycastHit hitInfo;
        if (Physics.Raycast(out hitInfo, (mTransform.Translation + new Vector3(1.5f, 0.0f, 0.0f)), new Vector3(1, 0, 0), 20.0f))
        {
            string tag = "";
            tag = FindEntityByID(hitInfo.EntityUUID).GetComponent<TagComponent>().Tag;
            if (tag != "")
                Console.LogInfo("The entity with name " + tag + " was hit by PhysX raycast!");
        }
    }
}
