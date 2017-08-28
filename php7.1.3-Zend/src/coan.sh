#!/bin/bash

cd Zend
for i in *
do
    coan source -U__cplusplus \
                -UZEND_WIN32 \
                -U_MSC_VER \
                -U_WIN64 \
                -U_WIN32 \
                -UNETWARE \
                -U__riscos__ \
                -UMAP_HUGETLB \
                -UZTS \
                -D__x86_64__ \
                -DZEND_DEBUG=0 \
                -DZEND_MM_LIMIT=1 \
                -DZEND_MM_STAT=1 \
                -DZEND_MM_ERROR=1 \
                -DZEND_MM_CUSTOM=0 \
                -DZEND_MM_STORAGE=0 \
                $i > ../my-Zend/$i
done
cd ../
