import serial
import csv
import datetime
import os
import time # Adicionado para a lógica de reconexão

# --- Configurações ---
DEFAULT_SERIAL_PORT = 'COM3'  # Atualizado para COM3
DEFAULT_BAUD_RATE = 115200
DEFAULT_CSV_FILENAME = 'dados_sensores.csv'

def get_serial_params():
    port = DEFAULT_SERIAL_PORT
    if not port:
        port = input(f"Digite a porta serial (ex: COM3, /dev/ttyUSB0): ")

    baud = DEFAULT_BAUD_RATE
    try:
        baud_input = input(f"Digite o baud rate (padrão {DEFAULT_BAUD_RATE}): ")
        if baud_input:
            baud = int(baud_input)
    except ValueError:
        print(f"Baud rate inválido. Usando padrão {DEFAULT_BAUD_RATE}.")
        # baud permanece como DEFAULT_BAUD_RATE
    return port, baud

def main():
    serial_port, baud_rate = get_serial_params()
    csv_filename = DEFAULT_CSV_FILENAME

    print(f"Tentando conectar a {serial_port} com baud rate {baud_rate}...")

    # Inicializa ser fora do try para que possa ser usado no finally
    ser = None

    try:
        ser = serial.Serial(serial_port, baud_rate, timeout=1)
        print(f"Conectado a {serial_port} com sucesso.")

        file_exists = os.path.isfile(csv_filename)

        with open(csv_filename, mode='a', newline='', encoding='utf-8') as csv_file:
            csv_writer = csv.writer(csv_file)

            if not file_exists:
                header = ['Timestamp_PC', 'Timestamp_ESP(ms)', 'Temperatura_C', 'Umidade_pct', 'Nivel_Audio_RMS']
                csv_writer.writerow(header)
                print(f"Arquivo {csv_filename} criado com cabeçalho.")
            else:
                print(f"Anexando dados ao arquivo {csv_filename}.")

            print("Pronto para receber dados. Pressione Ctrl+C para parar.")
            while True:
                try:
                    if ser.in_waiting > 0:
                        line = ser.readline().decode('utf-8').strip()

                        if line:
                            print(f"Recebido: {line}")
                            try:
                                esp_timestamp, temp, hum, audio = line.split(',')
                                pc_timestamp = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]

                                data_row = [
                                    pc_timestamp,
                                    esp_timestamp,
                                    float(temp),
                                    float(hum),
                                    float(audio)
                                ]
                                csv_writer.writerow(data_row)
                                # csv_file.flush() # Descomente para forçar escrita imediata

                            except ValueError as ve:
                                print(f"Erro ao processar dados: {line}. Erro: {ve}. Verifique o formato.")
                            except Exception as e:
                                print(f"Erro inesperado ao processar linha: {e}")

                except serial.SerialException as se:
                    print(f"Erro na comunicação serial: {se}")
                    print("Tentando reconectar...")
                    if ser and ser.is_open:
                        ser.close()
                    
                    # Loop de tentativa de reconexão
                    while True:
                        try:
                            time.sleep(2) # Espera 2 segundos antes de tentar novamente
                            print(f"Tentando reconectar à porta {serial_port}...")
                            ser = serial.Serial(serial_port, baud_rate, timeout=1)
                            print("Reconectado com sucesso.")
                            # Se reconectado, sair do loop de reconexão e continuar o loop principal
                            break 
                        except serial.SerialException:
                            print(".", end="") 
                        except KeyboardInterrupt: # Permite interromper a tentativa de reconexão
                            print("\nTentativa de reconexão interrompida pelo usuário.")
                            raise # Propaga a interrupção para sair do programa

                except KeyboardInterrupt:
                    print("\nPrograma interrompido pelo usuário.")
                    break
                except Exception as e:
                    print(f"Ocorreu um erro inesperado no loop principal: {e}")
                    break

    except serial.SerialException as e:
        print(f"Falha ao conectar à porta serial {serial_port}: {e}")
        print("Verifique se a porta está correta, se o dispositivo está conectado e se o baud rate coincide.")
    except IOError as e:
        print(f"Erro de E/S ao abrir/escrever no arquivo {csv_filename}: {e}")
    except KeyboardInterrupt: # Captura Ctrl+C durante a configuração inicial
        print("\nPrograma interrompido pelo usuário durante a inicialização.")
    except Exception as e:
        print(f"Ocorreu um erro não tratado na inicialização: {e}")
    finally:
        if ser and ser.is_open:
            ser.close()
            print("Porta serial fechada.")
        print("Programa finalizado.")

if __name__ == '__main__':
    main() 