using Electro;

class Player : ElectroAPI
{
    public void OnStart()
    {
        Console.LogDebug(null);
    }

    public void OnUpdate(float ts)
    {
        Console.LogDebug(ts);
    }
}