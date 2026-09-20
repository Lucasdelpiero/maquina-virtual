#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef void (*Operacion0)(Vmx *);
typedef void (*Operacion1)(Vmx *, int, int);
typedef void (*Operacion2)(Vmx *, int, int, int, int);

typedef union {
    Operacion0 op0;
    Operacion1 op1;
    Operacion2 op2;
} VOperacion;

const Minimo = 0x80000000;
const Maximo = 0xFFFFFFFF;


int validar_operacion(int CodOP ,int tipoA ,int tipoB) {
    int val = 0;
    if (tipoA >= 0 && tipoA <= 3)
      if (tipoB >= 0 && tipoB <= 3)
         if (CodOP >= 0 && CodOP <= 31)
            if (CodOP <= 10  || CodOP >= 15)
               val = 1;
    return val;
}

void mostrar_valor_SYS(uint8_t bytes[], int tamanio, int formato) {
    int i, j;
    uint32_t numero = 0;

    for (i = 0; i < tamanio; i++)
        numero |= ((uint32_t)bytes[i]) << (8 * i);

    if (formato & 0x10) {
        for (i = tamanio - 1; i >= 0; i--)
            for (j = 7; j >= 0; j--)
                printf("%d", (bytes[i] >> j) & 1);
    }

    if (formato & 0x08)
        printf("%X", numero);

    if (formato & 0x04)
        printf("%o", numero);

    if (formato & 0x02)
        for (i = 0; i < tamanio; i++)
            printf("%c", bytes[i]);

    if (formato & 0x01)
        printf("%u", numero);

    printf("\n");
}

uint32_t leer_numero_SYS(int formato) {
    uint32_t numero = 0;

    if (formato & 0x01)
        scanf("%u", &numero);

    else if (formato & 0x04)
        scanf("%o", &numero);

    else if (formato & 0x08)
        scanf("%x", &numero);

    else if (formato & 0x02) {
        char c;
        scanf(" %c", &c);
        numero = (uint8_t)c;
    }

    else if (formato & 0x10) {
        char binario[33];
        int i = 0;

        scanf("%32s", binario);

        while (binario[i] != '\0') {
            numero = numero << 1;

            if (binario[i] == '1')
                numero += 1;

            i++;
        }
    }

    return numero;
}

void SYS(Vmx *vmx, int tipoA, int datoA) {
    int32_t valorA = get_valor(vmx, tipoA, datoA);
    int32_t Resultado;

    int cantidad = (vmx->VRegistro[12] >> 16) & 0xFFFF;
    int tamanio = vmx->VRegistro[12] & 0xFFFF;
    int direccion = vmx->VRegistro[13];
    int formato = vmx->VRegistro[10];
    uint32_t numero;

    uint8_t bytes[tamanio];
    int i, j;

    if (valorA == 2) {
        valor = get_valor(vmx, 1 , direccion);
        for (i = 0; i < cantidad; i++) {
            for (j = 0; j < tamanio; j++)
                bytes[j] = (valor >> (8 * j)) & 0xFF;
            valor = valor >> 8 * tamanio;
            mostrar_valor_SYS(bytes, tamanio, formato);
        }
    }
    else 
        if (valorA == 1) {
            for (i = 0; i < cantidad; i++) {
                numero = leer_numero_SYS(formato);
                Resultado = (int32_t) numero;
                Guardar_Resultado(vmx , tipoA , datoA , Resultado);
        }
}

void realizar_salto(Vmx *vmx , int32_t Salto) {
    vmx->VRegistro[0] = Salto;
}

void JPM(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
        realizar_salto(vmx , valorA);
}

void JP(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
        if ((vmx->VRegistro[17] & 16) == 0 && (vmx->VRegistro[17] & 8) == 0)
            realizar_salto(vmx , valorA);
}

void JN(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
        if (vmx->VRegistro[17] & 16 && (vmx->VRegistro[17] & 8) == 0) 
            realizar_salto(vmx , valorA);
}

void JZ(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
        if ((vmx->VRegistro[17] & 16) == 0 && vmx->VRegistro[17] & 8)
            realizar_salto(vmx , valorA);
}

void JC(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
        if (vmx->VRegistro[17] & 2)
            realizar_salto(vmx , valorA);
}

void JV(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
        if (vmx->VRegistro[17] & 1)
            realizar_salto(vmx , valorA);
}

void JNP(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
        if (vmx->VRegistro[17] & 16 || vmx->VRegistro[17] & 8 )
            realizar_salto(vmx , valorA);
}

void JNN(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
        if ((vmx->VRegistro[17] & 16) == 0 )
            realizar_salto(vmx , valorA);
}

void JNZ(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
        if ((vmx->VRegistro[17] & 8) == 0  )
            realizar_salto(vmx , valorA);
}

void NOT(Vmx *vmx , int tipoA ,int datoA) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
    int32_t Resultado = ~valorA;
    int carry = 0 , desbordamiento = 0;
        ActualizarCC(vmx , Resultado , carry , desbordamiento);
        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void STOP(Vmx *vmx) {
    vmx->VRegistro[0] = -1;
}

void MOV(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t  valorB = get_valor(vmx, tipoB, datoB);
    int carry = 0 , desbordamiento = 0;
      ActualizarCC(vmx , valorB , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , valorB);
}


void ADD(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
    int32_t  valorB = get_valor(vmx, tipoB, datoB);
    int32_t Resultado = valorA + valorB ;
    int carry = ((uint64_t)(uint32_t)valorA + (uint32_t)valorB) > Maximo;
    int desbordamiento = (valorA > 0 && valorB > 0 && Resultado < 0) || (valorA < 0 && valorB < 0 && Resultado >= 0);
      ActualizarCC(vmx , Resultado , carry , desbordamiento);
      Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}


void SUB(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
    int32_t  valorB = get_valor(vmx, tipoB, datoB);
    int32_t Resultado = valorA - valorB ;
    int carry = (uint32_t)valorA < (uint32_t)valorB;
    int desbordamiento = (valorA >= 0 && valorB < 0 && Resultado < 0) || (valorA < 0 && valorB > 0 && Resultado >= 0);
      ActualizarCC(vmx , Resultado , carry , desbordamiento);
      Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void MUL(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
    int32_t  valorB = get_valor(vmx, tipoB, datoB);
    int32_t Resultado = valorA * valorB;
    int carry = ((uint64_t)(uint32_t)valorA * (uint32_t)valorB) >  Maximo;
    int desbordamiento = ((int64_t)valorA * valorB > Maximo || (int64_t)valorA * valorB < Minimo);
      ActualizarCC(vmx , Resultado , carry , desbordamiento);
       Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void DIV(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA  = get_valor(vmx, tipoA, datoA);
    int32_t  valorB = get_valor(vmx, tipoB, datoB);
    int32_t Resultado ;
    int carry = 0;
    int desbordamiento = valorA == Minimo && valorB == -1;
        if (valorB == 0)
            Detener_Ejecusion();
        Resultado = valorA / valorB ;
        vmx->VRegistro[16] = valorA % valorB;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void CMP(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t valorA = get_valor(vmx, tipoA, datoA);
    int32_t valorB = get_valor(vmx, tipoB, datoB);
    int32_t Resultado = valorA - valorB;
    int carry = (uint32_t)valorA < (uint32_t)valorB;
    int desbordamiento = ((valorA >= 0 && valorB < 0 && Resultado < 0) || (valorA < 0 && valorB > 0 && Resultado >= 0));
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

}

void AND(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
    int32_t  valorB = get_valor(vmx, tipoB, datoB);
    int32_t Resultado = valorA & valorB;
    int carry = 0 , desbordamiento = 0;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void OR(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
    int32_t  valorB = get_valor(vmx, tipoB, datoB);
    int32_t Resultado = valorA | valorB ;
    int carry = 0 , desbordamiento = 0;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void XOR(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
    int32_t  valorB = get_valor(vmx, tipoB, datoB);
    int32_t Resultado = valorA ^ valorB ;
    int carry = 0 , desbordamiento = 0;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void SWAP(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
    int32_t  valorB = get_valor(vmx, tipoB, datoB);
    int32_t Resultado = valorB;
    int32_t aux = valorA;
    int carry = 0 , desbordamiento = 0;
        ActualizarCC(vmx , Resultado , carry , desbordamiento);
        Guardar_Resultado(vmx , tipoA , datoA , valorB);
        Guardar_Resultado(vmx , tipoB , datoB , aux);
}

void SHL(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
    int32_t  valorB = get_valor(vmx, tipoB, datoB);
    int32_t Resultado ;
    int32_t carry = 0;
    int desbordamiento = ((int64_t)valorA * ((int64_t)1 << valorB) > Maximo || (int64_t)valorA * ((int64_t)1 << valorB) < Minimo);
    int i = 0;
        while (i < valorB && carry == 0) {
            if ((valorA >> (31 - i)) & 1)
                carry = 1;
                i++;
        }
        Resultado = valorA << valorB ;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void SHR(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
    int32_t  valorB = get_valor(vmx, tipoB, datoB);
    int32_t Resultado = 0;
    int  i , aux , n = 32 - valorB;
    int carry = 0 , desbordamiento = 0;
        i = 0;
        while (i < valorB && carry == 0) {
            if ((valorA >> (31 - i)) & 1)
                carry = 1;
                i++;
        }
        valorA = valorA >> valorB;
        for (i = 0 ; i < n ; i ++ ) {
            aux = valorA & 1;
            aux = aux << i;
            Resultado += aux;
            valorA = valorA >> 1;
        }
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void SAR(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
    int32_t  valorB = get_valor(vmx, tipoB, datoB);
    int32_t Resultado = 0;
    int  carry = 0 , desbordamiento = 0 , i;
        i = 0;
        while (i < valorB && carry == 0) {
            if ((valorA >> (31 - i)) & 1)
                carry = 1;
                i++;
        }
        Resultado = valorA >> valorB ;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void LDL(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
    int32_t  valorB = get_valor(vmx, tipoB, datoB);
    int32_t Resultado = 0;
    int  carry = 0 , desbordamiento = 0;
        Resultado = valorA >> valorB ;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void LDH(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
    int32_t  valorB = get_valor(vmx, tipoB, datoB);
    int32_t Resultado = 0;
    int  carry = 0 , desbordamiento = 0;
        Resultado = valorA >> valorB ;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);

        Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

void RND(Vmx *vmx , int tipoA ,int datoA , int tipoB , int datoB) {
    int32_t   valorA = get_valor(vmx, tipoA, datoA);
    int32_t  valorB = get_valor(vmx, tipoB, datoB);
    int32_t Resultado = rand() % (valorB + 1);;
    int  carry = 0 , desbordamiento = 0;
      ActualizarCC(vmx , Resultado , carry , desbordamiento);
      Guardar_Resultado(vmx , tipoA , datoA , Resultado);
}

VOperacion operaciones[32] = {
    [0x00].op1 = SYS,
    [0x01].op1 = JMP,
    [0x02].op1 = JP,
    [0x03].op1 = JN,
    [0x04].op1 = JZ,
    [0x05].op1 = JC,
    [0x06].op1 = JV,
    [0x07].op1 = JNP,
    [0x08].op1 = JNN,
    [0x09].op1 = JNZ,
    [0x0A].op1 = NOT,

    [0x0F].op0 = STOP,

    [0x10].op2 = MOV,
    [0x11].op2 = ADD,
    [0x12].op2 = SUB,
    [0x13].op2 = MUL,
    [0x14].op2 = DIV,
    [0x15].op2 = CMP,
    [0x16].op2 = AND,
    [0x17].op2 = OR,
    [0x18].op2 = XOR,
    [0x19].op2 = SWAP,
    [0x1A].op2 = SHL,
    [0x1B].op2 = SHR,
    [0x1C].op2 = SAR,
    [0x1D].op2 = LDL,
    [0x1E].op2 = LDH,
    [0x1F].op2 = RND
};

const char *mnemonicos[32] = {
    [0x00] = "SYS",
    [0x01] = "JMP",
    [0x02] = "JP",
    [0x03] = "JN",
    [0x04] = "JZ",
    [0x05] = "JC",
    [0x06] = "JV",
    [0x07] = "JNP",
    [0x08] = "JNN",
    [0x09] = "JNZ",
    [0x0A] = "NOT",

    [0x0F] = "STOP",

    [0x10] = "MOV",
    [0x11] = "ADD",
    [0x12] = "SUB",
    [0x13] = "MUL",
    [0x14] = "DIV",
    [0x15] = "CMP",
    [0x16] = "AND",
    [0x17] = "OR",
    [0x18] = "XOR",
    [0x19] = "SWAP",
    [0x1A] = "SHL",
    [0x1B] = "SHR",
    [0x1C] = "SAR",
    [0x1D] = "LDL",
    [0x1E] = "LDH",
    [0x1F] = "RND"
};

const char *registros[32] = {
    [0] = "IP",
    [1] = "OPC",
    [2] = "OP1",
    [3] = "OP2",
    [4] = "LAR",
    [5] = "MAR",
    [6] = "MBR",

    [10] = "EAX",
    [11] = "EBX",
    [12] = "ECX",
    [13] = "EDX",
    [14] = "EEX",
    [15] = "EFX",

    [16] = "AC",
    [17] = "CC",

    [26] = "CS",
    [27] = "DS"
};

void Mostrar_Operando(int tipo, int dato) {
    if (tipo == 1)
        printf("%s", registros[dato]);

    else if (tipo == 2)
        printf("[DS+%d]", dato);

    else if (tipo == 3)
        printf("%d", dato);
}


void Mostrar_Instruccion(Vmx *vmx, int CodOP, int tipoA, int datoA, int tipoB, int datoB) {

    int fin = vmx->registro[0];
    int i , aux ;
    int cantidad_bytes = 1 + tipoA + tipoB;
    int inicio = fin - 8 * cantidadBytes ;
    printf("[%04X] ", inicio);

    printf("  %02X ",(tipoA << 6) | (tipoB << 4) | (CodOP));

    if (tipoA == 3) {
       printf("%02X", datoA & 0xFF0);
       printf("%01x", datoA & 0x00F);
    }
    else
       if (tipoA == 2)
          printf("%02X",datoA);
        else
            if (tipoA == 1)
                printf("%01X",datoA);

    if (tipoB == 3) {
       printf("%02X", datoB & 0xFF0);
       printf("%01x", datoB & 0x00F);
    }
    else
       if (tipoB == 2)
          printf("%02X",datoB);
        else
            if (tipoB == 1)
                printf("%01X",datoB);
    
    for (i = cantidad_bytes; i < 8; i++)
        printf("   ");

    printf("|  %-5s", mnemonicos[CodOP]);

    if (CodOP <= 0x0A) {
        printf(" ");
        Mostrar_Operando(tipoA, datoA);
    }
    else if (CodOP >= 0x10) {
        printf(" ");
        Mostrar_Operando(tipoA, datoA);
        printf(", ");
        Mostrar_Operando(tipoB, datoB);
    }

    printf("\n");
}


void Realizar_Operacion(Vmx *vmx , VOperacion vo ,int CodOP ,int tipoA ,int tipoB) {
     if (!validar_operacion(CodOP , tipoA , tipoB) ) //agregar validar Datos
        Detener_Ejecusion();
    else
        if (CodOP <= 10)
            vo[CodOP].op1(vmx , tipoA,Get_Dato(*vmx,tipoA));
        else
           if (CodOP == 15)
                vo[CodOP].op0(vmx );
            else
                vo[CodOP].op2(vmx,tipoA, Get_Dato(*vmx,tipoA), tipoB,Get_Dato(*vmx,tipoB));
}
