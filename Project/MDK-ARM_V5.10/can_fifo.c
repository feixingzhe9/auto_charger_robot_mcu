#include "can_fifo.h"
#include <string.h>
#include <stdio.h>


static uint32_t can_fifo_surplus_size(can_fifo_t *head);
static uint8_t is_can_fifo_full(can_fifo_t *head);


uint8_t can_fifo_init(can_fifo_t *head, can_pkg_t *buf, uint32_t len)
{
    if(head == NULL)
    {
        return FALSE;
    }
    head->data = buf;
    head->size = len;
    head->front = head->rear = 0;

    return TRUE;
}


void can_fifo_rst(can_fifo_t *head)
{
    if(head == NULL)
    {
        return;
    }
    head->front = 0;
    head->rear = 0;
}


uint8_t is_can_fifo_empty(can_fifo_t *head)
{
    return ((head->front == head->rear) ? TRUE : FALSE);
}


static uint8_t is_can_fifo_full(can_fifo_t *head)
{
    return ((head->front == ((head->rear + 1) % head->size)) ? TRUE : FALSE);
}


uint32_t can_fifo_valid_size(can_fifo_t *head)
{
    return ((head->rear < head->front) ? (head->rear + head->size - head->front) : (head->rear - head->front));
}


uint8_t can_fifo_put_pkg(can_fifo_t *head, const can_pkg_t data)
{
    if(head == NULL)
    {
        return FALSE;
    }
    if(is_can_fifo_full(head) == TRUE)
    {
        return FALSE;
    }

    memcpy(&head->data[head->rear], &data, sizeof(can_pkg_t));
    head->rear++;
    head->rear = head->rear % head->size;

    return TRUE;
}


uint8_t can_fifo_get_pkg(can_fifo_t *head, can_pkg_t *data)
{
    if(head == NULL)
    {
        return FALSE;
    }
    if(is_can_fifo_empty(head) == TRUE)
    {
        return FALSE;
    }
    memcpy(data, &head->data[head->front], sizeof(can_pkg_t));
    head->front++;
    head->front = head->front % head->size;

    return TRUE;
}

