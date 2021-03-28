using UnityEngine;
using UnityEngine.EventSystems;

public class RotateRightController : MonoBehaviour, IPointerDownHandler, IPointerUpHandler
{
    public GameObject player;
    Rigidbody rb;
    private bool move = false;

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

    private void Update()
    {
        if (move)
        {
            rb.transform.Rotate(Vector3.up, 0.5f);
        }
        
    }
}