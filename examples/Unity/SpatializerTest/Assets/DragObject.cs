using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEditor;
using UnityEngine.SceneManagement;
using UnityEditor.SceneManagement;

namespace uOSC
{
    [CustomEditor(typeof(DragObject))]
    public class SourceEditor : Editor
    {
        uOscClient client = null;

        void OnEnable()
        {
            DragObject t = (target as DragObject);
            client = new uOscClient(t.ipAdress, t.port);
            //EditorSceneManager.activeSceneChangedInEditMode += ChangedActiveScene;
            
        }

        /*private void ChangedActiveScene(Scene current, Scene next)
        {


            Debug.Log("CHANGE");
        }*/

        void OnSceneGUI()
        {
            DragObject t = (target as DragObject);
            Vector3 cameraPos = SceneView.currentDrawingSceneView.camera.transform.position;
           // Vector3 listenerPos = t.listener.transform.position;
           // Vector3 listenerRot = t.listener.transform.rotation.eulerAngles;
            Vector3 direction = t.transform.position - cameraPos;

            //Debug.Log("x " + cameraPos.x);
            //Debug.Log("y " + cameraPos.y);
            //Debug.Log("z " + cameraPos.z);

            float azimuth = Mathf.Atan2(direction.x, direction.z) * 180/Mathf.PI;
            if (azimuth < 0)
                azimuth += 360;
           // if (listenerRot.y < 0)
               // listenerRot.y += 360;

           // azimuth += listenerRot.y;
            azimuth %= 360;
            float elevation = Mathf.Atan2(direction.y, Mathf.Sqrt(direction.x * direction.x + direction.z * direction.z)) * 180 / Mathf.PI;
            float distance = Vector3.Distance(t.transform.position, cameraPos);

            //Debug.Log("azimuth " + listenerRot.y);
            //Debug.Log("elevation " + elevation);
            //Debug.Log("distance " + distance);

            //Debug.Log("REPAINT");

           /* if (Event.current.type == EventType.Repaint)
            {
                client.ChangePort(t.port);
                client.Send(t.receiverName + "/azi", azimuth);
                client.Send(t.receiverName + "/ele", elevation);
                client.Send(t.receiverName + "/dist", distance);
                
            }*/

            if (Event.current.type == EventType.MouseDrag)
            {
                client.ChangePort(t.port);
                client.Send(t.receiverName + "/azi", azimuth);
                client.Send(t.receiverName + "/ele", elevation);
                client.Send(t.receiverName + "/dist", distance);
            }
        }
    }

    public class DragObject : MonoBehaviour
    {
        private Vector3 offset;
        private Vector3 lastPosition;
        private float x, y, z;
        private bool editZAxis = false;
        private bool mouseIsDragged = false;
        public uOscClient client;
        public GameObject listener;
        
        public float zScale = 0.3f;
        public string receiverName = "/uOSC";
        public string ipAdress;
        public int port;

        Vector3 GetMouseWorldPosition()
        {
            if (!editZAxis)
            {
                Vector3 screenPosition = Input.mousePosition;
                screenPosition.z = z;
                return Camera.main.ScreenToWorldPoint(screenPosition);
            }
            else
            {
                Vector3 screenPosition = Input.mousePosition;
                float delta = y - screenPosition.y * zScale;
                screenPosition.x = x;
                screenPosition.y = y;
                screenPosition.z += delta;
                return Camera.main.ScreenToWorldPoint(screenPosition);
            }
        }

        private void OnMouseDown()
        {
            x = Camera.main.WorldToScreenPoint(transform.position).x;
            y = Camera.main.WorldToScreenPoint(transform.position).y;
            z = Camera.main.WorldToScreenPoint(transform.position).z;

            offset = transform.position - GetMouseWorldPosition();
            mouseIsDragged = true;
            
        }

        private void OnMouseUp()
        {
            mouseIsDragged = false;
            editZAxis = false;
        }

        private void OnMouseDrag()
        {
            Vector3 newPosition = GetMouseWorldPosition() + offset;
            if (newPosition != transform.position)
            {
                transform.position = newPosition;
                client.Send(receiverName, transform.position.x, transform.position.y, transform.position.z);
            }
        }

        // Start is called before the first frame update
        void Awake()
        {
            client = new uOscClient(ipAdress, port);       
        }

        // Update is called once per frame
        void Update()
        {
            if (!mouseIsDragged)
            {
                if (Input.GetKeyDown(KeyCode.LeftControl))
                    editZAxis = true;
                if (Input.GetKeyUp(KeyCode.LeftControl))
                    editZAxis = false;
            }
        }
    }

    
}
