package app;

import javax.management.Notification;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Server implements Runnable{

    private ServerSocket server;

    // Connected clinents
    private List<ConnectionHandler> connections;

    // Offline users
    private java.util.Map<String, List<String>> offlineMessages = new java.util.concurrent.ConcurrentHashMap<>();


    // Working server
    private boolean working;

    // Thread pool of connections
    private ExecutorService pool;

    public Server(){
        connections = new CopyOnWriteArrayList<>();
        working = true;
    }

    private void startConsoleListener() {
        Thread consoleThread = new Thread(() -> {
            try (BufferedReader consoleReader = new BufferedReader(new InputStreamReader(System.in))) {
                while (working) {
                    String input = consoleReader.readLine();
                    if ("exit".equalsIgnoreCase(input)) {
                        log("Server shutdown initiated via console");
                        shutdown();
                        break;
                    }
                }
            } catch (IOException e) {
                log("Console listener error: " + e.getMessage());
            }
        });
        consoleThread.setDaemon(true);
        consoleThread.start();
    }

    @Override
    public void run() {
        try {

            // Setting server on port
            server = new ServerSocket(12345);
            log("Server started on port 12345");
            pool = Executors.newCachedThreadPool();

            // Start console listener for shutdown command
            startConsoleListener();

            while (working) {
                // Server accept connections
                Socket clientSocket = server.accept();
        
                // Adding connection
                ConnectionHandler handler = new ConnectionHandler(clientSocket);
                connections.add(handler);
                pool.execute(handler);
            }
                
            
        } catch (IOException e) {
            shutdown();
        }
    }
    
    
    // Shutdown Server
    public void shutdown(){
        try {
            working = false;
            pool.shutdown();
            if(!server.isClosed()){
                server.close();
            }

            // If server stop working all connections will shutdown
            for (ConnectionHandler connection : connections){
                log("Shutting down connection for: " + connection.nickName);
                connection.shutdown();
            }

        } catch (IOException e) {
            // Ingmoring exception
        }
    }

    // Inner Class that hanle connections
    class ConnectionHandler implements Runnable{

        private Socket clientSocket;
        // Getting information
        private BufferedReader in;
        // Sending information
        private PrintWriter out;

        // User nickName
        private String nickName;

        public ConnectionHandler(Socket clientSocket){
            this.clientSocket = clientSocket;
        }

        @Override
        public void run(){
            try {
                // Initializing in and out
                out = new PrintWriter(clientSocket.getOutputStream(), true);
                in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
                

                //out.println("Hello"); -> Sending information
                //in.readLine(); -> Reciving information

                // server entry login or register
                Registration();
                UnreadMessages();




                log(nickName + " connected!");
                broadcast(nickName + " join to server!");

                // Waiting for a new message
                String message;
                // Proccesing when message is not null
                while ((message = in.readLine()) != null){
                    // Checking commands
                    if (message.startsWith("/nick")){
                        log(nickName + " use commend /nick");
                        changeNickName(message);
                    }else if(message.startsWith("/quit")){
                        log(nickName + " use commend /quit");
                        broadcast(nickName + " left the server!");
                        log(nickName + " left the server!");
                        shutdown();
                        break;
                    }else if(message.startsWith("/help")){
                        log(nickName + " use commend /help");
                        helpCommand();
                    }else if (message.startsWith("/list")) {
                        log(nickName + " use commend /list");
                        listUsers();
                    }else if (message.startsWith("/msg")) {
                        privateMessage(message);
                    }else if(message.startsWith("/")) {
                        sendMessage("unknown command");
                    }else{
                        // Server sending message to all users from nickName user
                        broadcast(nickName + ": " + message);
                        log(nickName + " send message: " + message);
                    }
                }

            } catch (IOException e) {
                shutdown();
            }
        }
        public void helpCommand(){
            sendMessage("Available commands:");
            sendMessage(" /nick <newNickname> - Change your nickname.");
            sendMessage(" /quit - Quit the chat and disconnect from the server.");
            sendMessage(" /help - Show this help message.");
            sendMessage(" /list - Show list of online users.");
            sendMessage(" /msg <nickname> <message> - Send a private message to a specific user.");

        }

        private void shutdown(){
            try {
                connections.remove(this);
                in.close();
                out.close();
                if(!clientSocket.isClosed()){
                    log("Shutting down connection for: " + nickName);
                    clientSocket.close();
                }
            } catch (IOException e) {
                // Ingmoring exception
            }
        }

        // Sending message to all connections
        public void broadcast(String message){
            for(ConnectionHandler connection : connections){
                if(connection != null){
                    connection.sendMessage(message);
                }
            }
        }

        // Sending message
        public void sendMessage(String message){
            out.println(message);
        }

        // Changing nickName
        private void changeNickName(String message){
            String[] messageSplit = message.split(" ", 2);
            if (messageSplit.length == 2){
                String newNick = messageSplit[1];

                if (userExists(newNick)) {
                    sendMessage("This nickname is already taken.");
                    return;
                }

                if (updateNicknameInFile(nickName, newNick)) {
                    broadcast(nickName + " changed their nickname to " + newNick);
                    log(nickName + " changed their nickname to " + newNick);
                    sendMessage("You successfully changed your nickname to " + newNick);
                    nickName = newNick;
                } else {
                    sendMessage("Failed to change nickname.");
                }
            } else {
                sendMessage("Your input was bad. Usage: /nick <newNickname>");
                log(nickName + " failed to change nickname!");
            }
        }

        private boolean updateNicknameInFile(String oldNick, String newNick) {
            try {
                java.io.File file = new java.io.File("users.txt");
                java.util.List<String> lines = new java.util.ArrayList<>();
                String password = null;

                // Read users
                try (BufferedReader reader = new BufferedReader(new java.io.FileReader(file))) {
                    String line;
                    while ((line = reader.readLine()) != null) {
                        String[] parts = line.split(":");
                        if (parts[0].equals(oldNick)) {
                            password = parts[1]; // Save password
                            continue; // Skip old line
                        }
                        lines.add(line);
                    }
                }

                // If not found, return false
                if (password == null) return false;

                // Add new nickname with same password
                lines.add(newNick + ":" + password);

                // Write back all lines
                try (PrintWriter writer = new PrintWriter(new java.io.FileWriter(file))) {
                    for (String l : lines) writer.println(l);
                }

                return true;

            } catch (IOException e) {
                log("Error updating nickname in file: " + e.getMessage());
                return false;
            }
        }



        private void listUsers(){
            // Start building message
            StringBuilder message = new StringBuilder();

            message.append("Online users: ");
            for (ConnectionHandler connection : connections) {
                message.append("\n").append(connection.nickName);
            }

            // Send message
            sendMessage(message.toString());
            
        }

        private void UnreadMessages(){
            sendMessage("Unread messages:");
            if (offlineMessages.containsKey(nickName)) {
                List<String> pending = offlineMessages.get(nickName);
                for (String msg : pending) {
                    sendMessage(" [Offline] " + msg);
                }
                offlineMessages.remove(nickName);
                log("Delivered offline messages to " + nickName);
            }

        }

        private void privateMessage(String message) {
            String[] messageSplit = message.split(" ", 3);

            if (messageSplit.length < 3) {
                sendMessage("Invalid command. Use: /msg <username> <message>");
                return;
            }

            String targetNick = messageSplit[1];
            String privateMsg = messageSplit[2];


            log(nickName + " sent private message to " + targetNick);

            boolean found=false;
            for (ConnectionHandler connection : connections) {
                if (connection.nickName.equals(targetNick)) {
                    connection.sendMessage("[PM from " + nickName + "] " + privateMsg);
                    this.sendMessage("[PM to " + targetNick + "] " + privateMsg);
                    log(targetNick + " received private message from " + nickName);
                    found=true;
                    break;
                }
            }

            if (userExists(targetNick) && !found) {
                // User is offline, store message
                offlineMessages.putIfAbsent(targetNick, new java.util.ArrayList<>());
                offlineMessages.get(targetNick).add("[PM from " + nickName + "] " + privateMsg);
                this.sendMessage("User " + targetNick + " is offline. Message will be delivered when they return.");
                log("Stored offline message for " + targetNick + " from " + nickName);
            }else if(!found){
                this.sendMessage("User " + targetNick + " doesn't exist.");
            }
        }

        public class PasswordUtils {

            public String hashPassword(String password) {
                try {
                    MessageDigest md = MessageDigest.getInstance("SHA-256");
                    byte[] hashBytes = md.digest(password.getBytes());
                    StringBuilder sb = new StringBuilder();

                    for (byte b : hashBytes) {
                        sb.append(String.format("%02x", b)); // convert byte to hex
                    }

                    return sb.toString();
                } catch (NoSuchAlgorithmException e) {
                    throw new RuntimeException("Error hashing password", e);
                }
            }
        }


        private void Registration() {
            PasswordUtils passwordUtils=new PasswordUtils();
            while (true) {
                try {
                    out.println("Welcome! Type 1 to [Login] or 2 to [Register]: ");
                    String choice = in.readLine();

                    if ("1".equals(choice)) {
                        out.println("Enter username:");
                        String username = in.readLine();
                        out.println("Enter password:");
                        String plainPassword = in.readLine();
                        String hashedPassword = passwordUtils.hashPassword(plainPassword);

                        if (validateLogin(username, hashedPassword)) {
                            nickName = username;
                            out.println("Login successful! Welcome, " + nickName);
                            log(nickName + " logged in.");
                            break;
                        } else {
                            out.println("Login failed. Try again.");
                        }

                    } else if ("2".equals(choice)) {
                        out.println("Choose a username:");
                        String username = in.readLine();
                        out.println("Choose a password:");
                        String plainPassword = in.readLine();
                        String hashedPassword = passwordUtils.hashPassword(plainPassword);

                        if (registerUser(username, hashedPassword)) {
                            nickName = username;
                            out.println("Registration successful! Welcome, " + nickName);
                            log(nickName + " registered and joined.");
                            break;
                        } else {
                            out.println("Username already exists. Try again.");
                        }

                    } else {
                        out.println("Invalid choice. Type 1 to [Login] or 2 to [Register].");
                    }
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }


            }

        }

    }




    private boolean userExists(String username) {
        try (BufferedReader reader = new BufferedReader(new java.io.FileReader("users.txt"))) {
            String line;
            while ((line = reader.readLine()) != null) {
                String[] parts = line.split(":");
                if (parts[0].equals(username)) return true;
            }
        } catch (IOException e) {
            log("Error checking if user exists.");
        }
        return false;
    }

    private boolean registerUser(String username, String password) {
        if (userExists(username)) return false;

        try (PrintWriter writer = new PrintWriter(new java.io.FileWriter("users.txt", true))) {
            writer.println(username + ":" + password);
            return true;
        } catch (IOException e) {
            log("Error registering user.");
            return false;
        }
    }

    private boolean validateLogin(String username, String password) {
        try (BufferedReader reader = new BufferedReader(new java.io.FileReader("users.txt"))) {
            String line;
            while ((line = reader.readLine()) != null) {
                String[] parts = line.split(":");
                if (parts[0].equals(username) && parts[1].equals(password)) {
                    return true;
                }
            }
        } catch (IOException e) {
            log("Error validating login.");
        }
        return false;
    }





    private void log(String message) {
        String timestampedMessage = String.format("[%s] %s", java.time.LocalTime.now(), message);
        System.out.println(timestampedMessage);
        try (PrintWriter logWriter = new PrintWriter(new java.io.FileWriter("server.log", true))) {
            logWriter.println(timestampedMessage);
        } catch (IOException e) {
            System.out.println("Failed to write to server log.");
        }
    }



    // Running server
    public static void main(String[] args) {

        Server server = new Server();
        server.run();
        
    }
}
