#!/bin/bash
# VERSAO: 1.0
# NOME:	medir.qoe.sh
# DESCRICAO: Efetua a criação de video, chamada do simulador NS3 e avaliação de SSIM para QoE.
# NOTA:	etmp4, ffmpeg, mp4trace, MP4Box e MP4Box requerem o Wine para total funcionamento.
# AUTOR: Joahannes Costa <joahannes@gmail.com>.
#

caminho_video=video
caminho_resul=resultados
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
./waf --run scratch/v2x_wifi_normal
echo "---- FIM DA SIMULACAO ----"

printf "\n"
echo "------------------- CRIAÇÃO DO VIDEO RECEBIDO -------------------"
printf "\n"

for ((i=0; i<10; i++))
do
echo "Vídeo do Veiculo $i";
./$caminho_video/etmp4 -F -0 resultados/normal_sd_a01_$i resultados/normal_rd_a01_$i video/st_a03.st video/a03.mp4 videofinal_$i
done

printf "\n"
echo "------------------- GERAÇÃO DO ARQUIVO YUV PARA COMPARAÇÃO -------------------"
printf "\n"

for ((i=0; i<10; i++))
do
echo "Vídeo do Veiculo $i";
$caminho_video/ffmpeg -i 'videofinal_'$i'.mp4' 'videofinal'$i'.yuv'
done

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
mv videofinal* final/

printf "\n"
echo "------------------- GERANDO GRAFICOS ----------------------"
printf "\n"
#gnuplot graficos/Agrupamento_FlowVSThroughput.plt
#gnuplot graficos/Agrupamento_FlowVSDelay.plt
#gnuplot graficos/Agrupamento_FlowVSLostPackets.plt
#gnuplot graficos/Agrupamento_FlowVSJitter.plt

gnuplot graficos/Normal_FlowVSThroughput.plt
gnuplot graficos/Normal_FlowVSDelay.plt
gnuplot graficos/Normal_FlowVSLostPackets.plt
gnuplot graficos/Normal_FlowVSJitter.plt

printf "\n"
echo "------------------- FIM DA EXECUÇÃO DO SCRIPT -------------------"
printf "\n"
#EOF
