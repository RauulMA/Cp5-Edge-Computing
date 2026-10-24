#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Configurações - variáveis editáveis
const char* default_SSID = "Wokwi-GUEST"; // Nome da rede Wi-Fi
const char* default_PASSWORD = ""; // Senha da rede Wi-Fi
const char* default_BROKER_MQTT = "3.135.234.101"; // IP do Broker MQTT
const int default_BROKER_PORT = 1883; // Porta do Broker MQTT

// Tópicos MQTT
const char* default_TOPICO_SUBSCRIBE = "/TEF/device005/cmd"; 
const char* default_TOPICO_PUBLISH_1 = "/TEF/device005/attrs"; // Estado do LED
const char* default_TOPICO_PUBLISH_LUM = "/TEF/device005/attrs/l"; // Luminosidade
const char* default_TOPICO_PUBLISH_TEMP = "/TEF/device005/attrs/t"; // Temperatura
const char* default_TOPICO_PUBLISH_UMID = "/TEF/device005/attrs/h"; // Umidade

const char* default_ID_MQTT = "fiware_005"; // ID MQTT
const char* topicPrefix = "device005";

// Definição de Pinos no ESP32
const int PIN_LED = 2;   // LED Onboard (D4 / GPIO 2)
const int PIN_LDR = 34;  // Pino analógico para o LDR (GPIO 34)
const int PIN_DHT = 4;   // Pino digital para o DHT22 (GPIO 4)

#define DHTTYPE DHT22
DHT dht(PIN_DHT, DHTTYPE);

// Variáveis para configurações editáveis
char* SSID = const_cast<char*>(default_SSID);
char* PASSWORD = const_cast<char*>(default_PASSWORD);
char* BROKER_MQTT = const_cast<char*>(default_BROKER_MQTT);
int BROKER_PORT = default_BROKER_PORT;
char* TOPICO_SUBSCRIBE = const_cast<char*>(default_TOPICO_SUBSCRIBE);
char* TOPICO_PUBLISH_1 = const_cast<char*>(default_TOPICO_PUBLISH_1);
char* TOPICO_PUBLISH_LUM = const_cast<char*>(default_TOPICO_PUBLISH_LUM);
char* TOPICO_PUBLISH_TEMP = const_cast<char*>(default_TOPICO_PUBLISH_TEMP);
char* TOPICO_PUBLISH_UMID = const_cast<char*>(default_TOPICO_PUBLISH_UMID);
char* ID_MQTT = const_cast<char*>(default_ID_MQTT);

WiFiClient espClient;
PubSubClient MQTT(espClient);
char EstadoSaida = '0';

// Declarações de Funções
void initSerial();
void initWiFi();
void initMQTT();
void reconectWiFi();
void reconnectMQTT();
void VerificaConexoesWiFIEMQTT();
void mqtt_callback(char* topic, byte* payload, unsigned int length);
void EnviaEstadoOutputMQTT();
void InitOutput();
void handleLuminosity();
void handleTemperature();
void handleHumidity();

void setup() {
    InitOutput();
    initSerial();
    dht.begin();
    initWiFi();
    initMQTT();
    delay(5000);
    MQTT.publish(TOPICO_PUBLISH_1, "s|on");
}

void loop() {
    VerificaConexoesWiFIEMQTT();
    EnviaEstadoOutputMQTT();
    handleLuminosity();
    handleTemperature();
    handleHumidity();
    MQTT.loop();
    delay(2000); // Intervalo entre os ciclos de envio de dados dos sensores
}

void initSerial() {
    Serial.begin(115200);
}

void initWiFi() {
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    reconectWiFi();
}

void initMQTT() {
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);
    MQTT.setCallback(mqtt_callback);
}

void reconectWiFi() {
    if (WiFi.status() == WL_CONNECTED)
        return;
    WiFi.begin(SSID, PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println();
    Serial.println("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());

    // Garantir que o LED inicie desligado
    digitalWrite(PIN_LED, LOW);
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
    String msg;
    for (int i = 0; i < length; i++) {
        char c = (char)payload[i];
        msg += c;
    }
    Serial.print("- Mensagem recebida: ");
    Serial.println(msg);

    String onTopic = String(topicPrefix) + "@on|";
    String offTopic = String(topicPrefix) + "@off|";

    if (msg.equals(onTopic)) {
        digitalWrite(PIN_LED, HIGH);
        EstadoSaida = '1';
    }

    if (msg.equals(offTopic)) {
        digitalWrite(PIN_LED, LOW);
        EstadoSaida = '0';
    }
}

void VerificaConexoesWiFIEMQTT() {
    if (!MQTT.connected())
        reconnectMQTT();
    reconectWiFi();
}

void EnviaEstadoOutputMQTT() {
    if (EstadoSaida == '1') {
        MQTT.publish(TOPICO_PUBLISH_1, "s|on");
        Serial.println("- Led Ligado");
    } else {
        MQTT.publish(TOPICO_PUBLISH_1, "s|off");
        Serial.println("- Led Desligado");
    }
    Serial.println("- Estado do LED onboard enviado ao broker!");
}

void InitOutput() {
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, HIGH);
    boolean toggle = false;

    for (int i = 0; i <= 10; i++) {
        toggle = !toggle;
        digitalWrite(PIN_LED, toggle);
        delay(200);
    }
}

void reconnectMQTT() {
    while (!MQTT.connected()) {
        Serial.print("* Tentando se conectar ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) {
            Serial.println("Conectado com sucesso ao broker MQTT!");
            MQTT.subscribe(TOPICO_SUBSCRIBE);
        } else {
            Serial.println("Falha ao reconectar no broker.");
            Serial.println("Haverá nova tentativa de conexão em 2s");
            delay(2000);
        }
    }
}

void handleLuminosity() {
    int sensorValue = analogRead(PIN_LDR);
    int luminosity = map(sensorValue, 0, 4095, 100, 0); // Conversão para 0-100% Obs.: Trocar 0% e 100% de posição caso invertido
    char buffer[10];
    
    sprintf(buffer, "%d", luminosity);
    MQTT.publish(TOPICO_PUBLISH_LUM, buffer);
    Serial.print("Luminosidade: ");
    Serial.print(luminosity);
    Serial.println("%");
}

void handleTemperature() {
    float temperatura = dht.readTemperature();
    
    if (isnan(temperatura)) {
        Serial.println("Falha ao ler a temperatura do sensor DHT22!");
        return;
    }

    char buffer[10];
    dtostrf(temperatura, 4, 2, buffer);
    MQTT.publish(TOPICO_PUBLISH_TEMP, buffer);
    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.println(" °C");
}

void handleHumidity() {
    float umidade = dht.readHumidity();
    
    if (isnan(umidade)) {
        Serial.println("Falha ao ler a umidade do sensor DHT22!");
        return;
    }

    char buffer[10];
    dtostrf(umidade, 4, 2, buffer);
    MQTT.publish(TOPICO_PUBLISH_UMID, buffer);
    Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.println(" %");
}