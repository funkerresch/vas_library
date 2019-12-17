using UnityEngine;

namespace uOSC
{
    [RequireComponent(typeof(uOscClient))]
    public class ClientTest : MonoBehaviour
    {
        public string receiverName = "/uOSC/test";

        void Update()
        {
            float x = transform.position.x;

            var client = GetComponent<uOscClient>();
            client.Send(receiverName, 10, "hoge", "hogehoge", x, 123f);
        }
    }
}