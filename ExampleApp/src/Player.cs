using Electro;

class Player : Entity
{
    private RigidBodyComponent mRigidBody;
    public float mSpeed = 15.0f;

    public void OnStart()
    {
        mRigidBody = GetComponent<RigidBodyComponent>();
    }

    public void OnUpdate(float ts)
    {
        if (Input.IsKeyPressed(KeyCode.A))
            mRigidBody.AddForce(new Vector3(mSpeed, 0.0f, 0.0f), ForceMode.Acceleration);
        if (Input.IsKeyPressed(KeyCode.D))
            mRigidBody.AddForce(new Vector3(-mSpeed, 0.0f, 0.0f), ForceMode.Acceleration);
    }
}

class Ball : Entity
{
    private RigidBodyComponent mRigidBody;
    public float mBallSpeed = 10.0f;

    public void OnStart()
    {
        mRigidBody = GetComponent<RigidBodyComponent>();
        mRigidBody.AddForce(new Vector3(0.0f, 0.0f, -mBallSpeed), ForceMode.Impulse);
    }

}
