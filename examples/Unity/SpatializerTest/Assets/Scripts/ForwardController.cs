using UnityEngine;
using UnityEngine.EventSystems;

public class ForwardController : MonoBehaviour, IPointerDownHandler, IPointerUpHandler
{
    public GameObject player;
    Rigidbody rb;
    private bool move = false;
    float velocity = 5;

    public void OnPointerDown(PointerEventData eventData)
    {
        move = true;
    }
    public void OnPointerUp(PointerEventData eventData)
    {
        move = false;
    }

    private void Start()
    {
        rb = player.GetComponent<Rigidbody>();
    }

    public void GetVelocity(float value)
    {
        velocity = value;
    }

    private void Update()
    {
        if (move)
            rb.velocity = Camera.main.transform.forward * velocity;
        else
            rb.velocity = Vector3.zero;
    }
}