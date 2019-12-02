#lang racket/base

(require racket/list)
(require racket/stream)
(require racket/match)
(require racket/file)
(require racket/string)
(require rackunit)

(define OPCODE-ADD 1)
(define OPCODE-MUL 2)
(define OPCODE-STOP 99)
(define INSTRUCTION-WIDTH 4)

(define (opcode->fn opcode)
  (case opcode
    [(1) +]
    [(2) *]))

;; Processes the opcode at the start of `truncated-list`
;; and applies its result to `output-list`.
(define (process-opcode lst inx)
  (match (drop lst inx)
    [(list opcode lhs-inx rhs-inx out-inx _ ...)
     (append (take lst out-inx) ; lst until out-inx.
             ; The newly-calculated value:
             (list ((opcode->fn opcode)
                    (list-ref lst lhs-inx)
                    (list-ref lst rhs-inx)))
             (drop lst (add1 out-inx)) ; List after new out-inx.
             )]))

(define (eval-intcode lst [inx 0])
  (if (eq? OPCODE-STOP (list-ref lst inx))
      lst
      (eval-intcode (process-opcode lst inx)
                    (+ inx INSTRUCTION-WIDTH))))

(define (prepare-gravity-assist-intcode lst)
  (cons
   (car lst)
   (append
    '(12 02)
    (drop lst 3))))

(define (gravity-assist-input)
  (prepare-gravity-assist-intcode
   (map string->number
        (string-split
         (string-trim
          (file->string "data/day_2_gravity_assist_input.txt"))
         ","))))

(check-equal? (process-opcode '(1 0 0 0) 0) '(2 0 0 0))
(check-equal? (eval-intcode '(2 3 0 3 99))
              '(2 3 0 6 99))
(check-equal? (eval-intcode '(2 4 4 5 99 0))
              '(2 4 4 5 99 9801))
(check-equal? (eval-intcode '(1 1 1 4 99 5 6 0 99))
              '(30 1 1 4 2 5 6 0 99))
(check-equal? (car (eval-intcode (gravity-assist-input))) 6327510)