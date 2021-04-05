using Electro;

class Player : Entity
{
    private RigidBodyComponent mRigidBody;
    public float mSpeed = 10.0f;
    public void OnStart()
    {
        mRigidBody = GetComponent<RigidBodyComponent>();
    }

    public void OnUpdate(float ts)
    {
        if (Input.IsKeyPressed(KeyCode.D))
            mRigidBody.AddForce(new Vector3(mSpeed, 0.0f, 0.0f));
        if (Input.IsKeyPressed(KeyCode.A))
            mRigidBody.AddForce(new Vector3(-mSpeed, 0.0f, 0.0f));
        if (Input.IsKeyPressed(KeyCode.W))
            mRigidBody.AddForce(new Vector3(0.0f, 0.0f, -mSpeed));
        if (Input.IsKeyPressed(KeyCode.S))
            mRigidBody.AddForce(new Vector3(10.0f, 0.0f, mSpeed));

        if(Input.IsKeyPressed(KeyCode.Space))
            mRigidBody.AddForce(new Vector3(0.0f, 1.0f, 0.0f), ForceMode.Impulse);
    }
}
