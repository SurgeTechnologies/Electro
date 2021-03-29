using System;
using Spike;

class Enemy : Entity
{
    private TransformComponent m_EnemyTransform;
    private TransformComponent m_PlayerTransform;
    public void Start()
    {
        m_EnemyTransform = GetComponent<TransformComponent>();
        m_PlayerTransform = FindEntityByTag("Player").GetComponent<TransformComponent>();
        }

    public void Update(float ts)
    {
        var current = new Vector2(m_EnemyTransform.Translation.X, m_EnemyTransform.Translation.Y);
        var target = new Vector2(m_PlayerTransform.Translation.X, m_PlayerTransform.Translation.Y);
        m_EnemyTransform.Translation = new Vector3(Vector2.MoveTowards(current, target, 0.1f));
    }
}
