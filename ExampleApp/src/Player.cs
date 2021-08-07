using System;
using Electro;

class Player : Entity
{
    private RigidBodyComponent mRigidBody;
    public float mTorqueSpeed = 2.0f;
    public float mJumpSpeed = 2.0f;


    public void OnStart()
    {
        mRigidBody = GetComponent<RigidBodyComponent>();
        AddCollisionBeginCallback(OnCollisionBegin);
    }

    private void OnCollisionBegin(ulong other)
    {
        Entity entity = FindEntityByTag(other);
        Electro.Console.LogInfo("Player Collided With - " + entity.GetComponent<TagComponent>().Tag);
    }

    public void OnUpdate(float ts)
    {
        UpdateMovement();
    }

    private void UpdateMovement()
    {
        if (Input.IsKeyPressed(KeyCode.Space))
            mRigidBody.AddForce(new Vector3(0.0f, mJumpSpeed, 0.0f), ForceMode.Impulse);

        mRigidBody.AddTorque(new Vector3(0.0f, 0.0f, mTorqueSpeed));
    }
}
