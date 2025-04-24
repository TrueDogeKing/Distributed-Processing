# Makefile for Client-Server-Communication Java project

# Variables
PROJECT_NAME=Client-Server-Communication
JAR_NAME=target/$(PROJECT_NAME)-1.0-SNAPSHOT.jar
MAIN_SERVER=app.Server
MAIN_CLIENT=app.Client

# Default target
all: package

# Compile and package the project
package:
	mvn clean package

# Run server
server: package
	java -cp $(JAR_NAME) $(MAIN_SERVER)

# Run client
client: package
	java -cp $(JAR_NAME) $(MAIN_CLIENT)

# Clean target directory
clean:
	mvn clean

# Rebuild everything
rebuild: clean package

# Run tests
test:
	mvn test
