using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class NetworkManager : MonoBehaviour {

    private string serverAddress = "";
    private string serverState = "";

    public bool isServer { get; set; }

    public static NetworkManager instance = null;

	// Use this for initialization
	void Start () {
        instance = this;
	}
	
	// Update is called once per frame
	void Update () {
	
	}

    void OnGUI()
    {
        if (GUI.Button(new Rect(30, 100, 100, 50), "Start Server"))
        {
            Network.InitializeServer(32, 14000, false);
            isServer = true;
            Debug.Log("Start Server!");
        }
        serverAddress = GUI.TextField(new Rect(30, 20, 100, 30), "127.0.0.1");

        if (GUI.Button(new Rect(140, 100, 100, 50), "Connect Server"))
        {
            Debug.Log("Try Connect To Server!");
            isServer = false;
            Network.Connect(serverAddress, 14000);
        }

        serverState = GUI.TextField(new Rect(30, 60, 200, 30), "");
    }

    void OnServerInitialized()
    {
        serverState = "서버 활성화";
    }

    //  연결 실패
    void OnFailedToConnect(NetworkConnectionError error)
    {
        Debug.Log("Could not connect to server: " + error);
    }

    //  서버로의 클라이언트의 연결이 끊겼을 때
    void OnDisconnectedFromServer(NetworkDisconnection info)
    {
        Debug.Log("Disconnected from server: " + info);
    }

    //  클라이언트가 접속했을 때
    void OnPlayerConnected(NetworkPlayer player)
    {
        serverState = "접속 - " + player.ipAddress + ":" + player.port;
    }

    //  클라이언트가 접속을 끊었을 때
    void OnPlayerDisconnected(NetworkPlayer player)
    {
        Debug.Log("Player disconnected from " + player.ipAddress);
    }

}
