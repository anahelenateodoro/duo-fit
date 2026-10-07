# DuoFit

Jogo educativo 2D em C com Allegro 5, para Educação Física no ensino médio.
Projeto Integrador II · Senac. Equipe: Nathali, Ana Helena e Gabriel.
Entrega final: 30/11/2026.

## Executar

Com Allegro 5, compilador C e `pkg-config` instalados, execute na pasta do projeto:

```sh
./run.sh
```

O script compila `mainn.c`, `handebol.c` e `atletismo.c` juntos.
`mainn.c` contém o menu; cada esporte tem sua própria função de entrada
declarada em `fases.h`. `main.c` é o protótipo anterior.
A configuração atual de fontes usa Arial do macOS.

## Fase 2: Atletismo — Sprint 1

Responsável: Ana Helena Faustino Teodoro. Entrega prevista: 09/10/2026.
No menu, pressione **2** para entrar diretamente no atletismo.

- **Segure D ou →:** acelera para a frente até a velocidade máxima.
- **Solte:** desacelera até parar.
- **Espaço:** salta as barreiras (apenas quando está no chão).
- **E:** passa o bastão para Rico dentro da zona amarela, no chão.
- **R:** reinicia o percurso e o cronômetro.
- **Esc ou Backspace:** volta ao menu.

A Ana percorre uma pista de 100 metros com raias, marcações e linha de chegada.
A câmera acompanha a personagem e para perto do fim da pista.
O painel mostra distância, velocidade, tempo e progresso.
O cronômetro começa no primeiro movimento e para na chegada.

### Valores iniciais para ajustar no planejamento

A mecânica, o cenário e o loop da fase ficam em `atletismo.c`.
`atletismo.h` mantém apenas a declaração da função de entrada.
As constantes no início do arquivo controlam a corrida:

| Parâmetro | Valor |
| --- | --- |
| Escala | 40 pixels = 1 metro |
| Percurso | 4.000 pixels = 100 metros |
| Velocidade máxima | 520 pixels/s = 13 m/s |
| Aceleração | 260 pixels/s² = 6,5 m/s² |
| Desaceleração | 340 pixels/s² = 8,5 m/s² |
| Atualização | 60 vezes por segundo |

São valores de jogabilidade, para ajustar após experimentar.
A animação alterna `assets/ana_andando1.png` e `assets/ana_correndo2.png`
a cada 0,12 segundo enquanto Ana se move. A segunda pose tem um joelho
levantado e a outra perna dobrada para trás; os sprites originais foram preservados.
### Revezamento com barreiras

Percurso didático que combina duas mecânicas do atletismo, com seis barreiras.
Ana corre o primeiro trecho; Rico espera na zona amarela entre 45 e 55 metros.
Pressione **E** dentro da zona para assumir o Rico. Se não passar o bastão,
Ana espera no fim da zona. O segundo trecho termina na linha de chegada.
Bater em uma barreira a derruba, reduz a velocidade e conta uma batida.
Salte antes da barreira para passar por cima. **R** restaura os dois trechos
 e todas as barreiras. O percurso é uma adaptação para o jogo, não uma
reprodução de uma prova oficial combinando revezamento e barreiras.

Rico reaproveita seus sprites atuais de movimento e salto; ainda tem uma
única pose de corrida, enquanto Ana alterna duas poses.

### Conferência manual

1. Entre com **2** e confirme que Ana começa parada.
2. Segure **D**: confira o aumento da velocidade e a rolagem da pista.
3. Solte: confira que Ana desacelera e para.
4. Segure **→** até a chegada: confira a mensagem e o tempo congelado.
5. Pressione **R**: posição, câmera, velocidade e tempo devem voltar ao início.
6. Volte ao menu e selecione **HANDEBOL** com W/S e **Enter**.
