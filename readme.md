# Controlador PID

Projeto utilizado para aplicar um controlador PID em um sistema para controlar a temperatura. Com o auxilio do software PICSIMLab para simular a placa eletrônica Arduino UNO.

## Rodando o projeto

Na IDE do Arduino, ir na aba Sketch > Exportar binário compilado (Ctrl + Alt + S)

- Instalar a versão 0.8.10 do [PICSIMLab](https://sourceforge.net/projects/picsim/);
- Alterar para a placa Arduino Uno na aba "Board"; 
- Carregar o arquivo workspace.pzw ao PICSIMLab pela aba File > Load Workspace;
- Após isso ir em File > Load Hex, e ler o arquivo binário compilado anteriormente;
- Para visualizar os componentes pelo PICSIMLab acesse na aba Modules > Spare parts.

## Funcionamento

<img src="/assets/FuncionamentoPID.png" alt="FuncionamentoPID"/>
