# 🍷 Vinheria Agnello — Sistema IoT de Monitoramento Ambiental (ESP32 + MQTT)

**Checkpoint — Edge Computing & Computer Systems | FIAP 2026**

---

## 📋 Descrição do Projeto

Sistema de monitoramento ambiental inteligente desenvolvido com **ESP32** para a **Vinheria Agnello**. O objetivo é proteger o estoque de vinhos controlando rigorosamente as condições ambientais:
* **Luminosidade (LDR):** Evita a exposição excessiva à luz (que pode degradar os compostos orgânicos do vinho).
* **Temperatura e Umidade (DHT22):** Monitora o clima do ambiente para garantir que se mantenha dentro dos padrões ideais de armazenamento de vinhos.
* **Atuador e Telemetria (MQTT/FIWARE):** Permite o controle remoto do LED onboard (simulando um atuador de status) e o envio contínuo de dados para a nuvem via protocolo MQTT utilizando o padrão FIWARE Descomplicado.

---

## 🔧 Componentes Utilizados

| Componente | Quantidade | Descrição |
| :--- | :---: | :--- |
| **ESP32** | 1 | Microcontrolador principal com Wi-Fi integrado |
| **Sensor DHT22** | 1 | Sensor digital de temperatura e umidade |
| **Sensor LDR** | 1 | Sensor de luminosidade (Light Dependent Resistor) |
| **Resistor 330Ω** | 1 | Divisor de tensão para o LDR |
| **LED Vermelho** | 1 | LED para teste de conexão |
| **Protoboard** | 1 | Para montagem do circuito |
| **Jumpers** | Vários | Conexões elétricas |

---

## 🔌 Esquema de Ligação (Pinagem)

| Componente | Pino do ESP32 | Descrição |
| :--- | :---: | :--- |
| **LED Vermelho** | GPIO 2 (D4) | LED integrado para indicação de estado |
| **Sensor LDR** | GPIO 34 | Entrada analógica (ADC) para leitura de luz |
| **Sensor DHT22** | GPIO 4 | Entrada digital para dados de temperatura e umidade |

## 📦 Dependências (Bibliotecas Necessárias)

Para compilar e executar este projeto na Arduino IDE ou no Wokwi, certifique-se de instalar as seguintes bibliotecas:
* **WiFi** (Nativa do ESP32)
* **PubSubClient** (Por Nick O'Leary - para comunicação MQTT)
* **DHT sensor library** (Por Adafruit - para o sensor DHT22)
* **Adafruit Unified Sensor** (Dependência do DHT)

---

## ⚙️ Lógica de Funcionamento e Tópicos MQTT

O sistema conecta-se automaticamente à rede Wi-Fi e ao Broker MQTT configurado (`3.135.234.101`), publicando as leituras periodicamente:

* **Tópico de Comando (Subscribe):** `/TEF/device005/cmd` (Aguarda comandos `@on|` ou `@off|` para ligar/desligar o LED).
* **Tópico de Estado do LED (Publish):** `/TEF/device005/attrs` (Envia `s|on` ou `s|off`).
* **Tópico de Luminosidade (Publish):** `/TEF/device005/attrs/l` (Envia o valor percentual de 0 a 100%).
* **Tópico de Temperatura (Publish):** `/TEF/device005/attrs/t` (Envia a temperatura em °C).
* **Tópico de Umidade (Publish):** `/TEF/device005/attrs/h` (Envia a umidade relativa em %).

---

## 🚀 Como Reproduzir e Executar

### Simulação (Wokwi)
1. Acesse o Wokwi e crie um novo projeto para **ESP32**.
2. Adicione os componentes **LDR**, **Resistor de 10k**, e o **Sensor DHT22**.
3. Realize as conexões elétricas seguindo a tabela de pinagem descrita no projeto.
4. Instale as bibliotecas necessárias (`PubSubClient` e `DHT sensor library`) no gerenciador de bibliotecas.
5. Cole o código-fonte na aba principal e inicie a simulação.

### Hardware Físico & App MyMQTT
1. Abra a **Arduino IDE**.
2. Instale o suporte para placas ESP32 e as bibliotecas listadas na seção de dependências.
3. Conecte o ESP32 ao computador via USB, selecione a porta correta e faça o upload do código.
4. Para interagir via celular, utilize o aplicativo **MyMQTT**:
   * **Broker IP:** `3.135.234.101` | **Porta:** `1883`
   * **Inscreva-se (Subscribe) no tópico:** `/TEF/device005/#` para monitorar todos os dados em tempo real.
   * **Publique (Publish) no tópico:** `/TEF/device005/cmd` com a mensagem `device005@on|` ou `device005@off|` para controlar o LED remotamente.

---

## 💻 Código-Fonte

```cpp
// Autor original: Fábio Henrique Cabrini
// Atualizado para integrar: Controle de LED, Leitura de Luminosidade (LDR) e Sensor DHT22 (Temperatura e Umidade)
// Padrão de código modularizado por funções (FIWARE Descomplicado)

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Configurações - variáveis editáveis
const char* default_SSID = "Wokwi-GUEST"; // Nome da rede Wi-Fi
const char* default_PASSWORD = ""; // Senha da rede Wi-Fi
const char* default_BROKER_MQTT = "3.135.234.101"; // IP do Broker MQTT
const int default_BROKER_PORT = 1883; // Porta do Broker MQTT

// Tópicos MQTT (Padrão FIWARE Descomplicado)
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

// Declarações de Funções (Padrão do Professor)
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
    int luminosity = map(sensorValue, 0, 4095, 0, 100); // Conversão para 0-100%
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
