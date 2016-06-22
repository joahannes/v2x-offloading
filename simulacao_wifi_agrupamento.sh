#!/bin/bash
# VERSAO: 1.0 - 10/04/2016
# NOME:	simulacao_wifi_agrupamento.sh
# DESCRICAO: Efetua a criação de video, chamada do simulador NS3 e avaliação de SSIM para QoE.
# NOTA:	- etmp4, ffmpeg, mp4trace, x264 e MP4Box requerem o Wine para total funcionamento.
#	- configurar interface de rede para 192.168.0.1: ifconfig INTERFACE 192.168.0.1 netmask 255.255.255.0 up	
# AUTOR: Joahannes Costa <joahannes@gmail.com>.

#Variáveis com caminho para diretórios
caminho_video=video
caminho_resul=resultados/agrupamento
grafico=graficos/agrupamento

printf "\n"  
echo "------------------- CRIAÇÃO DO ARQUIVO DO VIDEO -------------------"
printf "\n"
x264 -I 10 --bframes 2 -B 300 --fps 30 -o $caminho_video/a03.264 --input-res 352x288 $caminho_video/container_cif.yuv

printf "\n"
echo "------------------- CRIAÇÃO DO VÍDEO -------------------"
printf "\n"
MP4Box -hint -mtu 1024 -fps 30 -add $caminho_video/a03.264 $caminho_video/a03.mp4

printf "\n"
echo "------------------- GERAÇÃO DO ST -------------------"
printf "\n"
./$caminho_video/mp4trace -f -s 192.168.0.2 12346 $caminho_video/a03.mp4 > $caminho_video/st_a03.st

printf "\n"
echo "------------------- EXECUÇÃO DA SIMULAÇÃO - NS3 -------------------"
printf "\n"
./waf --run scratch/v2x_wifi_agrupamento
echo "---- FIM DA SIMULACAO ----"

printf "\n"
echo "------------------- CRIAÇÃO DO VIDEO RECEBIDO -------------------"
printf "\n"

for ((i=0; i<9; i++))
do
echo "Vídeo do Veiculo $i";
./$caminho_video/etmp4 -F -0 $caminho_resul/sd_a01_$i $caminho_resul/rd_a01_$i video/st_a03.st video/a03.mp4 01_videofinal_$i
done

#Video do ClusterHead
echo "Vídeo do Veiculo CH";
./$caminho_video/etmp4 -F -0 $caminho_resul/sd_a01_CH $caminho_resul/rd_a01_CH video/st_a03.st video/a03.mp4 01_videofinal_CH

printf "\n"
echo "------------------- GERAÇÃO DO ARQUIVO YUV PARA COMPARAÇÃO -------------------"
printf "\n"

for ((i=0; i<9; i++))
do
echo "Vídeo do Veiculo $i";
./$caminho_video/ffmpeg -i '01_videofinal_'$i'.mp4' '01_videofinal_'$i'.yuv'
done

#Video do ClusterHead
echo "Vídeo do Veiculo CH";
./$caminho_video/ffmpeg -i '01_videofinal_CH.mp4' '01_videofinal_CH.yuv'

#printf "\n"
#echo "------------------- AVALIAÇÃO DE QoE - SSIM -------------------"
#path=pwd
#wine video/msu_metric.exe -f $caminho_video/container_cif.yuv IYUV -yw 352 -yh 288 -f 'videofinal_0.yuv' -sc 1 -cod $path -metr ssim_precise -cc YYUV

#DANDO PERMISSÃO AOS ARQUIVOS CRIADOS
chmod 777 $caminho_video/a03.264
chmod 777 $caminho_video/a03.mp4
chmod 777 $caminho_video/st_a03.st

rm delay*
rm loss*
rm rate*

#MOVENDO VIDEOS PARA PASTA final
mv 01_videofinal* final/agrupamento/

printf "\n"
echo "------------------- GERANDO GRAFICOS ----------------------"
printf "\n"
gnuplot $grafico/Agrupamento_FlowVSThroughput.plt
gnuplot $grafico/Agrupamento_FlowVSDelay.plt
gnuplot $grafico/Agrupamento_FlowVSLostPackets.plt
gnuplot $grafico/Agrupamento_FlowVSJitter.plt

printf "\n"
echo "------------------- FIM DA EXECUÇÃO DO SCRIPT -------------------"
printf "\n"
#EOF
