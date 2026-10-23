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

