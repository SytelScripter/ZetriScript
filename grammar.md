$$
\begin{align}
\text{PROGRAM} &\to \text{POS\_ACCESS}\space\text{STMT} \\
\text{STMT} &\to \text{POS\_ACCESS}:
    \begin{cases}
        \text{CLASSBUILTIN} \\
        \text{VAR\_ASSIGN} \\
        \text{EXEC} \\
    \end{cases}
\\
\text{POS\_ACCESS} &\to [\text{EXPR}:\text{EXPR}:\text{EXPR}] \\
\text{CLASSBUILTIN} &\to \text{KEYWORD}(
    \begin{cases}
        \text{POS\_ACCESS} \\
        \text{EXPR} \\
        \text{VAR\_ACCESS} \\
    \end{cases}
    ); \\
\text{VAR\_ASSIGN} &\to \text{IDENTIFIER} = 
    \begin{cases}
        \text{CLASSBUILTIN} \\
        \text{EXPR}
    \end{cases}; 
    \\
\text{VAR\_ACCESS} &\to \text{IDENTIFIER} \\
\text{EXEC} &\to 
    \begin{cases}
        \text{GOTO} \\
        \text{VAR\_ACCESS} \\
        \text{KEYWORD} \\
        \text{CLASSBUILTIN} \\
    \end{cases}
    ! \\
\text{GOTO} &\to goto\space\text{POS\_ACCESS}\\
\text{EXPR} &\to \text{TERM} 
    \begin{cases} 
        \text{PLUS} \\ 
        \text{MINUS} \\ 
    \end{cases}
\text{TERM} \\

\text{TERM} &\to \text{NUMBER} 
    \begin{cases} 
        \text{MUL} \\ 
        \text{DIV} \\ 
    \end{cases}
\text{NUMBER}
\end{align}
$$